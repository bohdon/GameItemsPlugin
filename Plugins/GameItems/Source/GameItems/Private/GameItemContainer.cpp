// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainer.h"

#include "GameItem.h"
#include "GameItemCollectionInterface.h"
#include "GameItemContainerDef.h"
#include "GameItemDef.h"
#include "GameItemSet.h"
#include "GameItemsModule.h"
#include "GameItemSubsystem.h"
#include "Algo/AnyOf.h"
#include "Engine/ActorChannel.h"
#include "Engine/Canvas.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Rules/GameItemAutoSlotRule.h"
#include "Rules/GameItemContainerLink.h"
#include "Rules/GameItemContainerRule.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainer)


// FGameItemContainerAddPlan
// -------------------------

void FGameItemContainerAddPlan::AddCountToSlot(int32 Slot, int32 Count)
{
	check(!TargetSlots.Contains(Slot));
	TargetSlots.Add(Slot);
	SlotDeltaCounts.Add(Count);
	DeltaCount += Count;
}

void FGameItemContainerAddPlan::UpdateDerivedValues(int32 ItemCount)
{
	check(TargetSlots.Num() == SlotDeltaCounts.Num());
	RemainderCount = ItemCount - DeltaCount;
	bWillAddFullAmount = DeltaCount > 0 && RemainderCount == 0;
	bWillSplit = TargetSlots.Num() > 1;
}


// UGameItemContainer
// ------------------

UGameItemContainer::UGameItemContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  ActiveChangeOperations(0),
	  NumSlotsPreChange(INDEX_NONE)
{
	ItemList.OnListChangedEvent.AddUObject(this, &UGameItemContainer::OnListChanged);
}

void UGameItemContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemList);
}

UObject* UGameItemContainer::GetItemOuter() const
{
	if (AActor* OwningActor = GetTypedOuter<AActor>())
	{
		return OwningActor;
	}
	return const_cast<UGameItemContainer*>(this);
}

void UGameItemContainer::SetContainerDef(TSubclassOf<UGameItemContainerDef> NewContainerDef)
{
	if (!ContainerDef)
	{
		ContainerDef = NewContainerDef;

		// duplicate container rules from the definition so that they can be stateful
		if (const UGameItemContainerDef* ContainerDefCDO = GetContainerDefCDO())
		{
			for (const UGameItemContainerRule* CDORule : ContainerDefCDO->Rules)
			{
				if (!CDORule)
				{
					continue;
				}

				UGameItemContainerRule* NewRule = DuplicateObject(CDORule, this);
				check(NewRule);
				Rules.Add(NewRule);
				NewRule->Initialize();
			}
		}
	}
}

void UGameItemContainer::SetCollection(TScriptInterface<IGameItemCollectionInterface> NewCollection)
{
	Collection = NewCollection;
}

const UGameItemContainerDef* UGameItemContainer::GetContainerDefCDO() const
{
	return ContainerDef ? GetDefault<UGameItemContainerDef>(ContainerDef) : nullptr;
}

FGameplayTagContainer UGameItemContainer::GetOwnedTags() const
{
	FGameplayTagContainer Result;
	Result.AddTag(ContainerId);
	if (ContainerDef)
	{
		Result.AppendTags(GetContainerDefCDO()->OwnedTags);
	}
	return Result;
}

FGameItemContainerAddPlan UGameItemContainer::CheckAddItem(UGameItem* Item, int32 TargetSlot, UGameItemContainer* OldContainer) const
{
	// when moving items within a collection, ignore collection limits
	const bool bIgnoreCollectionLimit = OldContainer && OldContainer->Collection == Collection;
	return GetAddItemPlan(Item, TargetSlot, bIgnoreCollectionLimit, false);
}

FGameItemContainerAddPlan UGameItemContainer::GetAddItemPlan(UGameItem* Item, int32 TargetSlot, bool bIgnoreCollectionLimit, bool bWarn) const
{
	FGameItemContainerAddPlan Plan;

	if (!Item || Contains(Item) || !CanContainItem(Item))
	{
		return Plan;
	}

	// get remaining space in the container
	int32 MaxDeltaCount = GetRemainingSpaceForItem(Item);

	// get remaining space in collection
	if (!bIgnoreCollectionLimit && !IsChild())
	{
		const int32 CollectionSpace = GetRemainingCollectionSpaceForItem(Item);
		MaxDeltaCount = FMath::Min(MaxDeltaCount, CollectionSpace);
	}

	// the total desired amount to add based on stock rules.
	// this doesn't include loss that may happen due from limited slots.
	const int32 DeltaCount = FMath::Min(Item->GetCount(), MaxDeltaCount);

	if (DeltaCount == 0)
	{
		UE_CLOG(bWarn, LogGameItems, Warning, TEXT("%s: Cant add item, max count reached: %s"),
		        *GetNameSafe(GetOwner()), *Item->ToDebugString());
		Plan.UpdateDerivedValues(Item->GetCount());
		return Plan;
	}

	if (DeltaCount < Item->GetCount())
	{
		UE_CLOG(bWarn, LogGameItems, Warning,
		        TEXT("%s: Adding %s, but %d will be lost due to capacity. Use CheckAddItem before adding to avoid this."),
		        *GetNameSafe(GetOwner()), *Item->ToDebugString(), Item->GetCount() - DeltaCount);
	}

	// repeatedly add the item, splitting and stacking as necessary
	int32 RemainingCountToAdd = DeltaCount;
	int32 NextTargetSlot = TargetSlot;

	const int32 StackMaxCount = GetItemStackMaxCount(Item);

	// if auto-stacking or dealing with limited slots, gather a list of matching items with space available
	TArray<UGameItem*> MatchingItemsWithSpace;
	if (GetContainerDefCDO()->bAutoStack || GetContainerDefCDO()->bLimitSlots)
	{
		MatchingItemsWithSpace = GetAllMatchingItems(Item);
		MatchingItemsWithSpace.RemoveAll([StackMaxCount](const UGameItem* MatchingItem)
		{
			return MatchingItem->GetCount() >= StackMaxCount;
		});
	}

	// track future number of empty slots as they are filled
	int32 NumEmptySlots = GetNumEmptySlots();
	while (RemainingCountToAdd > 0)
	{
		if (NumEmptySlots == 0 && MatchingItemsWithSpace.IsEmpty())
		{
			// out of space
			UE_CLOG(bWarn, LogGameItems, Warning,
			        TEXT("%s: Adding %s, but %d will be lost due to limited slot capacity. Use CheckAddItem before adding to avoid this."),
			        *GetNameSafe(GetOwner()), *Item->ToDebugString(), RemainingCountToAdd);
			break;
		}

		if (NextTargetSlot < 0)
		{
			// select a starting slot
			if (GetContainerDefCDO()->bAutoStack && !MatchingItemsWithSpace.IsEmpty())
			{
				// start with the first matching item
				NextTargetSlot = GetItemSlot(MatchingItemsWithSpace[0]);
				check(NextTargetSlot != INDEX_NONE);
			}
			else
			{
				NextTargetSlot = 0;
			}
		}

		// stack with existing items when auto stacking is enabled, or when there are no empty slots left
		const bool bCanStackWithExisting = GetContainerDefCDO()->bAutoStack || NumEmptySlots == 0;

		// attempt to add to next target slot
		UGameItem* ExistingItem = GetItemAt(NextTargetSlot);
		if (ExistingItem && bCanStackWithExisting && Item->IsMatching(ExistingItem) && ExistingItem->GetCount() < StackMaxCount)
		{
			// found matching item with space, add to it
			const int32 SlotDeltaCount = FMath::Min(RemainingCountToAdd, StackMaxCount - ExistingItem->GetCount());

			Plan.AddCountToSlot(NextTargetSlot, SlotDeltaCount);
			RemainingCountToAdd -= SlotDeltaCount;

			if (MatchingItemsWithSpace.Contains(ExistingItem))
			{
				MatchingItemsWithSpace.Remove(ExistingItem);
			}
		}
		else if (!ExistingItem)
		{
			// add to empty slot
			const int32 SlotDeltaCount = FMath::Min(RemainingCountToAdd, StackMaxCount);
			Plan.AddCountToSlot(NextTargetSlot, SlotDeltaCount);
			RemainingCountToAdd -= SlotDeltaCount;

			--NumEmptySlots;
		}

		// increment target slot and continue
		++NextTargetSlot;
	}

	Plan.UpdateDerivedValues(Item->GetCount());
	return Plan;
}

TArray<UGameItem*> UGameItemContainer::AddItem(UGameItem* Item, int32 TargetSlot)
{
	FScopedSlotChanges SlotChangeScope(this);

	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
	check(ItemSubsystem);

	FGameItemContainerAddPlan Plan = GetAddItemPlan(Item, TargetSlot);
	check(Plan.TargetSlots.Num() == Plan.SlotDeltaCounts.Num());

	TArray<UGameItem*> Result;

	for (int32 Idx = 0; Idx < Plan.TargetSlots.Num(); ++Idx)
	{
		const int32 Slot = Plan.TargetSlots[Idx];
		const int32 SlotDeltaCount = Plan.SlotDeltaCounts[Idx];

		if (UGameItem* ExistingItem = GetItemAt(Slot))
		{
			// increase count of existing item
			const int32 NewCount = ExistingItem->GetCount() + SlotDeltaCount;
			ExistingItem->SetCount(NewCount);
			Result.Add(ExistingItem);
		}
		else
		{
			// add the given item first, but if its already been added, duplicate and add a new item.
			UGameItem* NewItem = Item;
			if (Result.Contains(Item))
			{
				NewItem = ItemSubsystem->DuplicateItem(GetItemOuter(), Item);
			}
			NewItem->SetCount(SlotDeltaCount);

			ItemList.AddEntryAt(NewItem, Slot);
			NewItem->Containers.AddUnique(this);

			OnItemAdded(NewItem, Slot);
			OnSlotChanged(Slot);

			Result.Add(NewItem);
		}
	}

	return Result;
}

TArray<UGameItem*> UGameItemContainer::AddItems(TArray<UGameItem*> Items, int32 TargetSlot)
{
	if (Items.IsEmpty())
	{
		return TArray<UGameItem*>();
	}

	FScopedSlotChanges SlotChangeScope(this);

	TArray<UGameItem*> Result;
	for (UGameItem* Item : Items)
	{
		TArray<UGameItem*> AddResult = AddItem(Item, TargetSlot);
		Result.Append(AddResult);
	}
	return Result;
}

void UGameItemContainer::RemoveItem(UGameItem* Item)
{
	if (!Item)
	{
		return;
	}

	const int32 Index = GetItemSlot(Item);
	if (Index != INDEX_NONE)
	{
		RemoveItemAt(Index);
	}
}

void UGameItemContainer::RemoveItems(TArray<UGameItem*> Items)
{
	if (Items.IsEmpty())
	{
		return;
	}

	FScopedSlotChanges SlotChangeScope(this);
	for (UGameItem* Item : Items)
	{
		RemoveItem(Item);
	}
}

UGameItem* UGameItemContainer::RemoveItemAt(int32 Slot)
{
	if (!ItemList.Entries.IsValidIndex(Slot))
	{
		return nullptr;
	}

	FScopedSlotChanges SlotChangeScope(this);

	// don't preserve indexes for unlimited inventories
	const bool bPreserveIndexes = GetContainerDefCDO()->bLimitSlots;
	UGameItem* RemovedItem = ItemList.RemoveEntryAt(Slot, bPreserveIndexes);
	if (RemovedItem)
	{
		RemovedItem->Containers.Remove(this);

		OnItemRemoved(RemovedItem, Slot);
		OnSlotRangeChanged(Slot, bPreserveIndexes ? Slot : GetNumSlots() - 1);
	}

	return RemovedItem;
}

void UGameItemContainer::RemoveAllItems()
{
	// gather items that will be removed, and record which slot they were in
	int32 MaxSlot = 0;
	TMap<int32, UGameItem*> RemovedItems;
	for (int32 Slot = 0; Slot < ItemList.Entries.Num(); ++Slot)
	{
		if (UGameItem* Item = ItemList.Entries[Slot].GetItem())
		{
			RemovedItems.Add(Slot, Item);
			MaxSlot = FMath::Max(MaxSlot, Slot);
		}
	}

	if (RemovedItems.IsEmpty())
	{
		// nothing to do
		return;
	}

	FScopedSlotChanges SlotChangeScope(this);

	ItemList.Reset();

	for (const auto& Elem : RemovedItems)
	{
		const int32 Slot = Elem.Key;
		UGameItem* Item = Elem.Value;

		Item->Containers.Remove(this);
		OnItemRemoved(Item, Slot);
	}

	OnSlotRangeChanged(0, MaxSlot);
}

void UGameItemContainer::SwapItems(int32 SlotA, int32 SlotB)
{
	if (!IsValidSlot(SlotA) || !IsValidSlot(SlotB) || SlotA == SlotB)
	{
		return;
	}

	FScopedSlotChanges SlotChangeScope(this);

	ItemList.SwapEntries(SlotA, SlotB);
	OnSlotsChanged({SlotA, SlotB});
}

void UGameItemContainer::StackItems(int32 FromSlot, int32 ToSlot, bool bAllowPartial)
{
	UGameItem* FromItem = GetItemAt(FromSlot);
	UGameItem* ToItem = GetItemAt(ToSlot);

	if (!FromItem || !ToItem)
	{
		return;
	}

	if (!FromItem->IsMatching(ToItem))
	{
		return;
	}

	const int32 OldCount = ToItem->GetCount();
	const int32 MaxStackCount = GetItemStackMaxCount(ToItem);
	const int32 MaxDeltaCount = FMath::Max(MaxStackCount - OldCount, 0);

	const int32 DeltaCount = FMath::Min(FromItem->GetCount(), MaxDeltaCount);

	if (DeltaCount == 0)
	{
		// cant add item, max stack count reached
		return;
	}

	if (DeltaCount < FromItem->GetCount() && !bAllowPartial)
	{
		// cant stack the full amount
		return;
	}

	if (DeltaCount == FromItem->GetCount())
	{
		// remove all of the item
		RemoveItemAt(FromSlot);
	}
	else
	{
		// remove delta
		FromItem->SetCount(FromItem->Count - DeltaCount);
	}

	// add delta
	ToItem->SetCount(ToItem->GetCount() + DeltaCount);
}

TArray<UGameItem*> UGameItemContainer::GetAllItems() const
{
	TArray<UGameItem*> Result;
	ItemList.GetAllItems(Result);
	return Result;
}

UGameItem* UGameItemContainer::GetItemAt(int32 Slot) const
{
	return ItemList.Entries.IsValidIndex(Slot) ? ItemList.Entries[Slot].GetItem() : nullptr;
}

UGameItem* UGameItemContainer::GetFirstItem() const
{
	return GetItemAt(0);
}

UGameItem* UGameItemContainer::FindFirstItemByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		UGameItem* EntryItem = Entry.GetItem();
		if (IsValid(EntryItem) && EntryItem->GetItemDef() == ItemDef)
		{
			return EntryItem;
		}
	}
	return nullptr;
}

UGameItem* UGameItemContainer::FindFirstMatchingItem(const UGameItem* Item) const
{
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		UGameItem* EntryItem = Entry.GetItem();
		if (IsValid(EntryItem) && EntryItem->IsMatching(Item))
		{
			return EntryItem;
		}
	}
	return nullptr;
}

TArray<UGameItem*> UGameItemContainer::GetAllMatchingItems(const UGameItem* Item) const
{
	TArray<UGameItem*> Result;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		UGameItem* EntryItem = Entry.GetItem();
		if (IsValid(EntryItem) && EntryItem->IsMatching(Item))
		{
			Result.Add(EntryItem);
		}
	}
	return Result;
}

int32 UGameItemContainer::GetItemSlot(const UGameItem* Item) const
{
	return ItemList.Entries.IndexOfByPredicate([Item](const FGameItemListEntry& Entry)
	{
		return Entry.GetItem() == Item;
	});
}

bool UGameItemContainer::Contains(const UGameItem* Item) const
{
	return GetItemSlot(Item) != INDEX_NONE;
}

int32 UGameItemContainer::GetTotalItemCountByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	int32 Total = 0;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		const UGameItem* EntryItem = Entry.GetItem();
		if (IsValid(EntryItem) && EntryItem->GetItemDef() == ItemDef)
		{
			Total += EntryItem->GetCount();
		}
	}
	return Total;
}

int32 UGameItemContainer::GetTotalMatchingItemCount(const UGameItem* Item) const
{
	int32 Total = 0;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		const UGameItem* EntryItem = Entry.GetItem();
		if (IsValid(EntryItem) && EntryItem->IsMatching(Item))
		{
			Total += EntryItem->GetCount();
		}
	}
	return Total;
}

int32 UGameItemContainer::GetCollectionMatchingItemCount(const UGameItem* Item) const
{
	const IGameItemCollectionInterface* CollectionInterface = Collection.GetInterface();
	return CollectionInterface ? CollectionInterface->GetTotalMatchingItemCount(Item) : GetTotalMatchingItemCount(Item);
}

int32 UGameItemContainer::GetTotalItemCount() const
{
	int32 Total = 0;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		const UGameItem* EntryItem = Entry.GetItem();
		if (IsValid(EntryItem))
		{
			Total += EntryItem->GetCount();
		}
	}
	return Total;
}

bool UGameItemContainer::IsStackFull(int32 Slot) const
{
	if (const UGameItem* Item = GetItemAt(Slot))
	{
		const int32 MaxStackCount = GetItemStackMaxCount(Item);
		return Item->GetCount() >= MaxStackCount;
	}
	return false;
}

int32 UGameItemContainer::GetNumItems() const
{
	int32 Total = 0;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (Entry.GetItem())
		{
			++Total;
		}
	}
	return Total;
}

bool UGameItemContainer::IsEmpty() const
{
	return GetNumItems() == 0;
}

int32 UGameItemContainer::GetNumSlots() const
{
	return GetContainerDefCDO()->bLimitSlots ? GetContainerDefCDO()->SlotCount : GetNumItems();
}

bool UGameItemContainer::IsSlotCountLimited() const
{
	return GetContainerDefCDO()->bLimitSlots;
}

int32 UGameItemContainer::GetNumEmptySlots() const
{
	// always return zero when no space is left, even if container was overfilled somehow
	return GetContainerDefCDO()->bLimitSlots ? FMath::Max(GetContainerDefCDO()->SlotCount - GetNumItems(), 0) : INDEX_NONE;
}

int32 UGameItemContainer::GetNextEmptySlot() const
{
	if (GetContainerDefCDO()->bLimitSlots)
	{
		// iterate over limited slot count
		for (int32 Idx = 0; Idx < GetContainerDefCDO()->SlotCount; ++Idx)
		{
			if (IsSlotEmpty(Idx))
			{
				return Idx;
			}
		}
		return INDEX_NONE;
	}

	// iterate over existing entries looking for any gaps
	for (int32 Idx = 0; Idx < ItemList.Entries.Num(); ++Idx)
	{
		if (IsSlotEmpty(Idx))
		{
			return Idx;
		}
	}
	// return next index
	return ItemList.Entries.Num() + 1;
}

bool UGameItemContainer::IsValidSlot(int32 Slot) const
{
	return Slot >= 0 && (!GetContainerDefCDO()->bLimitSlots || Slot < GetContainerDefCDO()->SlotCount);
}

bool UGameItemContainer::IsSlotEmpty(int32 Slot) const
{
	return !ItemList.Entries.IsValidIndex(Slot) || ItemList.Entries[Slot].GetItem() == nullptr;
}

bool UGameItemContainer::CanContainItem(const UGameItem* Item) const
{
	if (!Item)
	{
		return false;
	}

	const UGameItemDef* ItemDefCDO = Item->GetItemDefCDO();
	if (!ItemDefCDO)
	{
		return false;
	}

	for (const UGameItemContainerRule* Rule : Rules)
	{
		if (!Rule->CanContainItem(Item))
		{
			return false;
		}
	}
	return true;
}

int32 UGameItemContainer::GetItemMaxCount(const UGameItem* Item) const
{
	const UGameItemDef* ItemDefCDO = Item ? Item->GetItemDefCDO() : nullptr;
	if (!ItemDefCDO)
	{
		return 0;
	}

	// find the smallest stack limit as defined by stock rules
	int32 Result = ItemDefCDO->ContainerLimit.GetMaxCount();

	for (const UGameItemContainerRule* Rule : Rules)
	{
		const int32 RuleMaxCount = Rule->GetItemMaxCount(Item);
		if (RuleMaxCount >= 0)
		{
			Result = FMath::Min(Result, RuleMaxCount);
		}
	}

	return Result;
}

int32 UGameItemContainer::GetItemStackMaxCount(const UGameItem* Item) const
{
	const UGameItemDef* ItemDefCDO = Item ? Item->GetItemDefCDO() : nullptr;
	if (!ItemDefCDO)
	{
		return 0;
	}

	// find the smallest stack limit as defined by stock rules
	int32 Result = ItemDefCDO->StackLimit.GetMaxCount();

	for (const UGameItemContainerRule* Rule : Rules)
	{
		const int32 RuleMaxCount = Rule->GetItemStackMaxCount(Item);
		if (RuleMaxCount >= 0)
		{
			Result = FMath::Min(Result, RuleMaxCount);
		}
	}

	return Result;
}

int32 UGameItemContainer::GetItemCollectionMaxCount(const UGameItem* Item) const
{
	const UGameItemDef* ItemDefCDO = Item ? Item->GetItemDefCDO() : nullptr;
	if (!ItemDefCDO)
	{
		return 0;
	}

	return ItemDefCDO->CollectionLimit.GetMaxCount();
}

int32 UGameItemContainer::GetRemainingSpaceForItem(const UGameItem* Item) const
{
	return FMath::Max(GetItemMaxCount(Item) - GetTotalMatchingItemCount(Item), 0);
}

int32 UGameItemContainer::GetRemainingCollectionSpaceForItem(const UGameItem* Item) const
{
	return FMath::Max(GetItemCollectionMaxCount(Item) - GetCollectionMatchingItemCount(Item), 0);
}

void UGameItemContainer::AddDefaultItems(bool bForce)
{
	if (bHasDefaultItems && !bForce)
	{
		return;
	}

	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
	for (const FGameItemDefStack& DefaultItem : GetContainerDefCDO()->DefaultItems)
	{
		ItemSubsystem->CreateItemInContainer(this, DefaultItem.ItemDef, DefaultItem.Count);
	}

	for (const UGameItemSet* ItemSet : GetContainerDefCDO()->DefaultItemSets)
	{
		if (!ItemSet)
		{
			continue;
		}

		ItemSet->AddToContainer(this);
	}

	if (!GetContainerDefCDO()->DefaultDropContent.IsNull())
	{
		FGameItemDropContext Context;
		Context.TargetActor = GetOwner();

		const TArray<UGameItem*> NewItems = ItemSubsystem->CreateItemsFromDropTable(this, Context, GetContainerDefCDO()->DefaultDropContent);
		AddItems(NewItems);
	}

	bHasDefaultItems = true;
}

UGameItemContainerRule* UGameItemContainer::GetRule(TSubclassOf<UGameItemContainerRule> RuleClass) const
{
	for (UGameItemContainerRule* Rule : Rules)
	{
		if (Rule->IsA(RuleClass))
		{
			return Rule;
		}
	}
	return nullptr;
}

UGameItemContainerRule* UGameItemContainer::AddRule(TSubclassOf<UGameItemContainerRule> RuleClass)
{
	UGameItemContainerRule* NewRule = NewObject<UGameItemContainerRule>(this, RuleClass);
	if (NewRule)
	{
		Rules.Add(NewRule);
		NewRule->Initialize();
		return NewRule;
	}
	return nullptr;
}

int32 UGameItemContainer::RemoveRule(TSubclassOf<UGameItemContainerRule> RuleClass)
{
	TArray<UGameItemContainerRule*> MatchingRules = Rules.FilterByPredicate([RuleClass](const UGameItemContainerRule* Rule)
	{
		check(Rule);
		return Rule->GetClass() == RuleClass;
	});

	int32 NumRemoved = 0;
	for (UGameItemContainerRule* Rule : MatchingRules)
	{
		Rule->Uninitialize();
		Rules.Remove(Rule);
		++NumRemoved;
	}
	return NumRemoved;
}

bool UGameItemContainer::IsChild() const
{
	return Algo::AnyOf(Rules, [](const UGameItemContainerRule* Rule) { return Rule->IsChild(); });
}

bool UGameItemContainer::HasParent(UGameItemContainer* ParentContainer) const
{
	return Algo::AnyOf(Rules, [ParentContainer](const UGameItemContainerRule* Rule)
	{
		const UGameItemContainerLink* LinkRule = Cast<UGameItemContainerLink>(Rule);
		return LinkRule && LinkRule->IsChild() && LinkRule->GetLinkedContainer() == ParentContainer;
	});
}

TArray<UGameItemContainer*> UGameItemContainer::GetChildren() const
{
	return ChildContainers;
}

void UGameItemContainer::RegisterChild(UGameItemContainer* ChildContainer)
{
	if (ChildContainer)
	{
		ChildContainers.AddUnique(ChildContainer);
	}
}

void UGameItemContainer::UnregisterChild(UGameItemContainer* ChildContainer)
{
	ChildContainers.Remove(ChildContainer);
}

int32 UGameItemContainer::GetAutoSlotPriorityForItem(UGameItem* Item, FGameplayTagContainer ContextTags) const
{
	if (!Item)
	{
		return 0;
	}

	int32 Priority = 0;
	for (const UGameItemContainerRule* Rule : Rules)
	{
		if (const UGameItemAutoSlotRule* AutoSlotRule = Cast<UGameItemAutoSlotRule>(Rule))
		{
			Priority = FMath::Max(Priority, AutoSlotRule->GetAutoSlotPriorityForItem(Item, ContextTags));
		}
	}
	return Priority;
}

bool UGameItemContainer::CanAutoSlot(UGameItem* Item, FGameplayTagContainer ContextTags) const
{
	for (const UGameItemContainerRule* Rule : Rules)
	{
		if (const UGameItemAutoSlotRule* AutoSlotRule = Cast<UGameItemAutoSlotRule>(Rule))
		{
			if (AutoSlotRule->CanAutoSlot(Item, ContextTags))
			{
				// at least 1 rule can handle the auto-slot
				return true;
			}
		}
	}
	return false;
}

TArray<UGameItem*> UGameItemContainer::TryAutoSlot(UGameItem* Item, FGameplayTagContainer ContextTags)
{
	if (!Item)
	{
		return TArray<UGameItem*>();
	}

	for (const UGameItemContainerRule* Rule : Rules)
	{
		if (const UGameItemAutoSlotRule* AutoSlotRule = Cast<UGameItemAutoSlotRule>(Rule))
		{
			if (AutoSlotRule->CanAutoSlot(Item, ContextTags))
			{
				TArray<UGameItem*> OutItems;
				if (AutoSlotRule->TryAutoSlot(Item, ContextTags, OutItems))
				{
					return OutItems;
				}
			}
		}
	}
	return TArray<UGameItem*>();
}

UGameItemContainer* UGameItemContainer::FindAutoSlotChildContainerForItem(UGameItem* Item, FGameplayTagContainer ContextTags) const
{
	UGameItemContainer* BestContainer = nullptr;
	int32 BestPriority = -1;

	for (UGameItemContainer* Container : ChildContainers)
	{
		check(Container);
		const int32 Priority = Container->GetAutoSlotPriorityForItem(Item, ContextTags);
		if (!BestContainer || Priority > BestPriority)
		{
			BestContainer = Container;
			BestPriority = Priority;
		}
	}
	return BestContainer;
}

AActor* UGameItemContainer::GetOwner() const
{
	return GetTypedOuter<AActor>();
}

UWorld* UGameItemContainer::GetWorld() const
{
	if (GetOuter())
	{
		return GetOuter()->GetWorld();
	}
	return nullptr;
}

void UGameItemContainer::CommitSaveData(FGameItemContainerSaveData& ContainerData, TMap<UGameItem*, FGuid>& SavedItems)
{
	// serialize all items
	bool bIsChild = IsChild();
	TArray<UGameItem*> Items = GetAllItems();
	ContainerData.ItemList.Reset();
	for (UGameItem* Item : Items)
	{
		int32 Slot = GetItemSlot(Item);
		check(Slot != INDEX_NONE);

		if (bIsChild)
		{
			// store only guid pointing to parent item
			if (FGuid* ItemGuid = SavedItems.Find(Item))
			{
				ContainerData.ItemList.Emplace(Slot, *ItemGuid);
			}
		}
		else
		{
			// serialize item data
			const FGameItemSaveData& ItemData = ContainerData.ItemList.Emplace(Slot, Item);

			// store item guid for children to access
			SavedItems.Add(Item, ItemData.Guid);
		}
	}

	// serialize additional container data
	FMemoryWriter MemWriter(ContainerData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	Serialize(Ar);
}

void UGameItemContainer::LoadSaveData(const FGameItemContainerSaveData& ContainerData, TMap<FGuid, UGameItem*>& LoadedItems)
{
	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);

	RemoveAllItems();

	// load items
	bool bIsChild = IsChild();
	for (const auto& ItemElem : ContainerData.ItemList)
	{
		int32 Slot = ItemElem.Key;
		const FGameItemSaveData& ItemData = ItemElem.Value;

		if (bIsChild)
		{
			// look for existing item loaded by parent
			if (UGameItem* LoadedItem = LoadedItems.FindRef(ItemData.Guid))
			{
				AddItem(LoadedItem, Slot);
			}
		}
		else
		{
			// create new item from save data
			if (ItemData.ItemDef.IsNull())
			{
				UE_LOG(LogGameItems, Warning, TEXT("Found null item def when loading save game: %s.%d"), *ContainerId.ToString(), Slot);
				continue;
			}

			const TSubclassOf<UGameItemDef> ItemDef = ItemData.ItemDef.LoadSynchronous();
			if (!ItemDef)
			{
				UE_LOG(LogGameItems, Warning, TEXT("Failed to load item def when loading save game: %s"), *ItemData.ItemDef.ToString());
				continue;
			}

			UGameItem* NewItem = ItemSubsystem->CreateItem(GetOwner(), ItemDef);
			check(NewItem);

			// save item so it can be retrieved by children
			LoadedItems.Add(ItemData.Guid, NewItem);

			// serialize item properties
			FMemoryReader MemReader(ItemData.ByteData);
			FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
			Ar.ArIsSaveGame = true;
			NewItem->Serialize(Ar);

			AddItem(NewItem, Slot);
		}
	}

	// load serialized properties
	FMemoryReader MemReader(ContainerData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;
	Serialize(Ar);
}

void UGameItemContainer::OnItemAdded(UGameItem* Item, int32 Slot)
{
	check(Item);

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s OnItemAdded [%d] %s"), *GetName(), Slot, *GetNameSafe(Item));
	OnItemAddedEvent.Broadcast(Item);
	Item->OnSlottedEvent.Broadcast(this, Slot, INDEX_NONE);

	// if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Item)
	// {
	// 	AddReplicatedSubObject(Item);
	// }
}

void UGameItemContainer::OnItemRemoved(UGameItem* Item, int32 Slot)
{
	check(Item);

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s OnItemRemoved [%d] %s"), *GetName(), Slot, *GetNameSafe(Item));
	OnItemRemovedEvent.Broadcast(Item);
	Item->OnUnslottedEvent.Broadcast(this, Slot);

	// if (IsUsingRegisteredSubObjectList() && Item)
	// {
	// 	RemoveReplicatedSubObject(Item);
	// }
}

void UGameItemContainer::BeginSlotChanges()
{
	if (ActiveChangeOperations == 0)
	{
		NumSlotsPreChange = GetNumSlots();
	}
	++ActiveChangeOperations;
}

void UGameItemContainer::EndSlotChanges()
{
	--ActiveChangeOperations;
	check(ActiveChangeOperations >= 0);

	if (ActiveChangeOperations == 0)
	{
		BroadcastSlotChanges();
	}
}

void UGameItemContainer::BroadcastSlotChanges()
{
	struct FSlotRange
	{
		FSlotRange()
		{
		}

		FSlotRange(int32 Slot)
			: Start(Slot), End(Slot)
		{
		}

		int32 Start = -1;
		int32 End = -1;

		bool IsValid() const
		{
			return Start != -1;
		}

		bool Adjoins(const int32& Next) const
		{
			return Next == End + 1;
		}
	};

	auto BroadcastRange = [this](const FSlotRange& Range)
	{
		if (Range.Start == Range.End)
		{
			UE_LOG(LogGameItems, VeryVerbose, TEXT("%s OnItemSlotChanged %d"), *GetName(), Range.Start);
			OnItemSlotChangedEvent.Broadcast(Range.Start);
		}
		else
		{
			UE_LOG(LogGameItems, VeryVerbose, TEXT("%s OnItemSlotsChanged %d - %d"), *GetName(), Range.Start, Range.End);
			OnItemSlotsChangedEvent.Broadcast(Range.Start, Range.End);
		}
	};

	// aggregate into adjacent ranges
	FSlotRange CurrentRange;
	for (int32 Idx = 0; Idx < ChangedSlots.Num(); ++Idx)
	{
		const int32& Slot = ChangedSlots[Idx];
		if (!CurrentRange.IsValid())
		{
			CurrentRange = Slot;
		}
		else if (CurrentRange.Adjoins(Slot))
		{
			CurrentRange.End = Slot;
		}
		else
		{
			BroadcastRange(CurrentRange);

			// start a new range
			CurrentRange = Slot;
		}

		if (Idx == ChangedSlots.Num() - 1)
		{
			BroadcastRange(CurrentRange);
		}
	}
	ChangedSlots.Empty();

	if (NumSlotsPreChange != INDEX_NONE)
	{
		const int32 NewNumSlots = GetNumSlots();
		if (NumSlotsPreChange != NewNumSlots)
		{
			UE_LOG(LogGameItems, VeryVerbose, TEXT("%s OnNumSlotsChanged %d -> %d"), *GetName(), NumSlotsPreChange, NewNumSlots);
			OnNumSlotsChangedEvent.Broadcast(NewNumSlots, NumSlotsPreChange);
		}
	}
}

void UGameItemContainer::OnSlotChanged(int32 Slot)
{
	ChangedSlots.Add(Slot);
}

void UGameItemContainer::OnSlotsChanged(const TArray<int32>& Slots)
{
	for (const int32& Slot : Slots)
	{
		ChangedSlots.Add(Slot);
	}
}

void UGameItemContainer::OnSlotRangeChanged(int32 StartSlot, int32 EndSlot)
{
	for (int32 Slot = StartSlot; Slot <= EndSlot; ++Slot)
	{
		ChangedSlots.Add(Slot);
	}
}

void UGameItemContainer::OnListChanged(FGameItemListEntry& Entry, int32 NewCount, int32 OldCount)
{
	// re-broadcast the change from this component
	// TODO
}

void UGameItemContainer::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) const
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::White);
	const FString CountStr = GetContainerDefCDO()->bLimitSlots
		                         ? FString::Printf(TEXT("%d/%d"), GetNumItems(), GetContainerDefCDO()->SlotCount)
		                         : FString::FromInt(GetNumItems());
	DisplayDebugManager.DrawString(FString::Printf(TEXT("%s (%s items)"), *ContainerId.ToString(), *CountStr));

	for (int32 Idx = 0; Idx < ItemList.Entries.Num(); ++Idx)
	{
		const FGameItemListEntry& Entry = ItemList.Entries[Idx];
		DisplayDebugManager.DrawString(FString::Printf(TEXT("    [%d] %s"), Idx, *Entry.ToDebugString()));
	}
}
