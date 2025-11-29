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
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Rules/GameItemAutoSlotRule.h"
#include "Rules/GameItemContainerLink.h"
#include "Rules/GameItemContainerRule.h"
#include "Serialization/MappedName.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif

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
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		ItemList.OnPostReplicateChangesEvent.AddUObject(this, &UGameItemContainer::OnPostReplicatedChanges);
	}
}

void UGameItemContainer::SetContainerId(FGameplayTag NewContainerId)
{
	if (ContainerId != NewContainerId)
	{
		ContainerId = NewContainerId;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ContainerId, this);
	}
}

void UGameItemContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ContainerId, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, DisplayName, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ContainerDef, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Rules, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ChildContainers, Params);

	DOREPLIFETIME(ThisClass, ItemList);
}

int32 UGameItemContainer::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}
	// specifically check for player state, and if found use player controller, to avoid
	// issues where player state is SimulatedProxy, and the PC is the AutonomousProxy
	if (const APlayerState* PlayerState = GetTypedOuter<APlayerState>())
	{
		if (APlayerController* PC = PlayerState->GetPlayerController())
		{
			return PC->GetFunctionCallspace(Function, Stack);
		}
	}
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UGameItemContainer::CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter() != nullptr);

	AActor* Actor = GetTypedOuter<AActor>();
	UNetDriver* NetDriver = Actor ? Actor->GetNetDriver() : nullptr;
	if (!NetDriver)
	{
		return false;
	}
	NetDriver->ProcessRemoteFunction(Actor, Function, Parms, OutParms, Stack, this);
	return true;
}

#if UE_WITH_IRIS
void UGameItemContainer::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UObject::RegisterReplicationFragments(Context, RegistrationFlags);

	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif

FString UGameItemContainer::GetNetDebugString() const
{
	if (ensure(GetWorld()))
	{
		switch (GetWorld()->GetNetMode())
		{
		case NM_DedicatedServer:
		case NM_ListenServer:
			return TEXT("Server: ");
		case NM_Client:
			return FString::Printf(TEXT("Client %d: "), UE::GetPlayInEditorID());
		case NM_Standalone:
		default: ;
		}
	}
	return FString();
}

UObject* UGameItemContainer::GetItemOuter() const
{
	// use actor for replication, items are duplicated into containers when moving between actors
	if (AActor* OwningActor = GetTypedOuter<AActor>())
	{
		return OwningActor;
	}
	return const_cast<UGameItemContainer*>(this);
}

void UGameItemContainer::SetContainerDef(TSubclassOf<UGameItemContainerDef> NewContainerDef)
{
	if (!ensureMsgf(!ContainerDef, TEXT("Cannot change ContainerDef after creation")))
	{
		return;
	}

	ContainerDef = NewContainerDef;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ContainerDef, this);

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
			NewRule->ClearFlags(RF_AllFlags);

			Rules.Add(NewRule);
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Rules, this);

			OnRuleAdded(NewRule);
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
		UE_CLOG(bWarn, LogGameItems, Warning, TEXT("%s[%s] Cant add item, max count reached: %s"),
		        *GetNetDebugString(), *GetReadableName(), *Item->GetDebugString());
		Plan.UpdateDerivedValues(Item->GetCount());
		return Plan;
	}

	if (DeltaCount < Item->GetCount())
	{
		UE_CLOG(bWarn, LogGameItems, Warning,
		        TEXT("%s[%s] Adding %s, but %d will be lost due to capacity. Use CheckAddItem before adding to avoid this."),
		        *GetNetDebugString(), *GetReadableName(), *Item->GetDebugString(), Item->GetCount() - DeltaCount);
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
			        TEXT("%s[%s] Adding %s, but %d will be lost due to limited slot capacity. Use CheckAddItem before adding to avoid this."),
			        *GetNetDebugString(), *GetReadableName(), *Item->GetDebugString(), RemainingCountToAdd);
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

void UGameItemContainer::AddItem(UGameItem* Item, int32 TargetSlot)
{
	if (!HasAuthority())
	{
		ServerAddItem(Item, TargetSlot);
		if (!CanExecuteLocally())
		{
			return;
		}
	}

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

			ItemList.AddEntryForSlot(NewItem, Slot);

			OnItemAdded(NewItem, Slot);
			OnSlotChanged(Slot);

			Result.Add(NewItem);
		}
	}
}

void UGameItemContainer::AddItems(TArray<UGameItem*> Items, int32 TargetSlot)
{
	if (!HasAuthority())
	{
		ServerAddItems(Items, TargetSlot);
		if (!CanExecuteLocally())
		{
			return;
		}
	}

	if (Items.IsEmpty())
	{
		return;
	}

	FScopedSlotChanges SlotChangeScope(this);

	for (UGameItem* Item : Items)
	{
		AddItem(Item, TargetSlot);
	}
}

void UGameItemContainer::RemoveItem(UGameItem* Item)
{
	if (!HasAuthority())
	{
		ServerRemoveItem(Item);
		if (!CanExecuteLocally())
		{
			return;
		}
	}

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
	if (!HasAuthority())
	{
		ServerRemoveItems(Items);
		if (!CanExecuteLocally())
		{
			return;
		}
	}

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

void UGameItemContainer::RemoveItemAt(int32 Slot)
{
	if (!HasAuthority())
	{
		ServerRemoveItemAt(Slot);
		if (!CanExecuteLocally())
		{
			return;
		}
	}

	if (!ItemList.HasItemInSlot(Slot))
	{
		return;
	}

	FScopedSlotChanges SlotChangeScope(this);

	// slide all items after this down when the inventory has unlimited slots (remove gaps)
	const bool bShouldCollapse = !GetContainerDefCDO()->bLimitSlots;
	UGameItem* RemovedItem = ItemList.RemoveEntryForSlot(Slot, bShouldCollapse);
	if (RemovedItem)
	{
		OnItemRemoved(RemovedItem, Slot);
		OnSlotRangeChanged(Slot, bShouldCollapse ? GetNumSlots() - 1 : Slot);
	}
}

void UGameItemContainer::RemoveItemsByDef(TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	if (!HasAuthority())
	{
		ServerRemoveItemsByDef(ItemDef, Count);
		if (!CanExecuteLocally())
		{
			return;
		}
	}

	if (!ItemDef)
	{
		return;
	}
	Count = FMath::Max(Count, 0);

	int32 NumRemoved = 0;
	TArray<UGameItem*> Items = FindItemsByDef(ItemDef);
	for (UGameItem* Item : Items)
	{
		const int32 NumToRemove = Count - NumRemoved;
		if (NumToRemove == 0)
		{
			break;
		}

		NumRemoved += FMath::Min(Item->GetCount(), NumToRemove);
		const int32 NewCount = Item->GetCount() - NumRemoved;
		if (NewCount > 0)
		{
			Item->SetCount(NewCount);
		}
		else
		{
			RemoveItem(Item);
		}
	}
}

void UGameItemContainer::RemoveAllItems()
{
	if (!HasAuthority())
	{
		ServerRemoveAllItems();
		if (!CanExecuteLocally())
		{
			return;
		}
	}

	// gather items that will be removed, and record which slot they were in
	int32 MaxSlot = 0;
	TMap<int32, UGameItem*> RemovedItems;
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		if (UGameItem* Item = Entry.Item)
		{
			RemovedItems.Add(Entry.Slot, Item);
			MaxSlot = FMath::Max(MaxSlot, Entry.Slot);
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

		OnItemRemoved(Item, Slot);
	}

	OnSlotRangeChanged(0, MaxSlot);
}

void UGameItemContainer::SwapItems(int32 SlotA, int32 SlotB)
{
	if (!HasAuthority())
	{
		ServerSwapItems(SlotA, SlotB);
		if (!CanExecuteLocally())
		{
			return;
		}
	}

	if (!IsValidSlot(SlotA) || !IsValidSlot(SlotB) || SlotA == SlotB)
	{
		return;
	}

	FScopedSlotChanges SlotChangeScope(this);

	// nothing will change if both slots are empty
	if (ItemList.SwapEntries(SlotA, SlotB))
	{
		OnSlotsChanged({SlotA, SlotB});
	}
}

void UGameItemContainer::StackItems(int32 FromSlot, int32 ToSlot, bool bAllowPartial)
{
	if (!HasAuthority())
	{
		ServerStackItems(FromSlot, ToSlot);
		if (!CanExecuteLocally())
		{
			return;
		}
	}

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

TMap<int32, UGameItem*> UGameItemContainer::GetAllItems() const
{
	TMap<int32, UGameItem*> Result;
	ItemList.GetAllItems(Result);
	return Result;
}

TArray<UGameItem*> UGameItemContainer::GetAllItemsAsSlotArray() const
{
	TArray<UGameItem*> Result;
	Result.SetNum(GetNumSlots());

	TMap<int32, UGameItem*> ItemsBySlot;
	ItemList.GetAllItems(ItemsBySlot);

	for (int32 Slot = 0; Slot < Result.Num(); ++Slot)
	{
		Result[Slot] = ItemsBySlot.FindRef(Slot);
	}
	return Result;
}

UGameItem* UGameItemContainer::GetItemAt(int32 Slot) const
{
	return ItemList.GetItemInSlot(Slot);
}

UGameItem* UGameItemContainer::GetFirstItem() const
{
	return GetItemAt(0);
}

UGameItem* UGameItemContainer::FindFirstItemByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	if (!ItemDef)
	{
		return nullptr;
	}
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		UGameItem* EntryItem = Entry.Item;
		if (IsValid(EntryItem) && EntryItem->GetItemDef() == ItemDef)
		{
			return EntryItem;
		}
	}
	return nullptr;
}

TArray<UGameItem*> UGameItemContainer::FindItemsByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	TArray<UGameItem*> Result;
	if (!ItemDef)
	{
		return Result;
	}
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		UGameItem* EntryItem = Entry.Item;
		if (IsValid(EntryItem) && EntryItem->GetItemDef() == ItemDef)
		{
			Result.Add(EntryItem);
		}
	}
	return Result;
}

UGameItem* UGameItemContainer::FindFirstItemByTag(FGameplayTagContainer RequireTags, FGameplayTagContainer IgnoreTags) const
{
	if (RequireTags.IsEmpty() && IgnoreTags.IsEmpty())
	{
		return nullptr;
	}
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		UGameItem* EntryItem = Entry.Item;
		if (IsValid(EntryItem) && EntryItem->GetOwnedTags().HasAll(RequireTags) && !EntryItem->GetOwnedTags().HasAny(IgnoreTags))
		{
			return EntryItem;
		}
	}
	return nullptr;
}

TArray<UGameItem*> UGameItemContainer::FindItemsByTag(FGameplayTagContainer RequireTags, FGameplayTagContainer IgnoreTags) const
{
	TArray<UGameItem*> Result;
	if (RequireTags.IsEmpty() && IgnoreTags.IsEmpty())
	{
		return Result;
	}
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		UGameItem* EntryItem = Entry.Item;
		if (IsValid(EntryItem) && EntryItem->GetOwnedTags().HasAll(RequireTags) && !EntryItem->GetOwnedTags().HasAny(IgnoreTags))
		{
			Result.Add(EntryItem);
		}
	}
	return Result;
}

UGameItem* UGameItemContainer::FindFirstMatchingItem(const UGameItem* Item) const
{
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		UGameItem* EntryItem = Entry.Item;
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
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		UGameItem* EntryItem = Entry.Item;
		if (IsValid(EntryItem) && EntryItem->IsMatching(Item))
		{
			Result.Add(EntryItem);
		}
	}
	return Result;
}

int32 UGameItemContainer::GetItemSlot(const UGameItem* Item) const
{
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		if (Entry.Item.Get() == Item)
		{
			return Entry.Slot;
		}
	}
	return INDEX_NONE;
}

void UGameItemContainer::SetItemAt(UGameItem* Item, int32 Slot)
{
	if (!HasAuthority())
	{
		ServerSetItemAt(Item, Slot);
		if (!CanExecuteLocally())
		{
			return;
		}
	}

	if (GetItemAt(Slot) != Item)
	{
		RemoveItemAt(Slot);
		AddItem(Item, Slot);
	}
}

bool UGameItemContainer::Contains(const UGameItem* Item) const
{
	return GetItemSlot(Item) != INDEX_NONE;
}

int32 UGameItemContainer::GetTotalItemCountByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	if (!ItemDef)
	{
		return 0;
	}

	int32 Total = 0;
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		const UGameItem* EntryItem = Entry.Item;
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
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		const UGameItem* EntryItem = Entry.Item;
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
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		const UGameItem* EntryItem = Entry.Item;
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
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		if (Entry.Item)
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
	TArray<int32> FilledSlots;
	ItemList.GetAllSlots(FilledSlots);

	// filled slots may not be sequential (1, 2, 4, 5),
	// but we know if we iterate for the same count we will find any gaps,
	// and if none, return the new highest slot
	for (int32 Slot = 0; Slot < FilledSlots.Num(); ++Slot)
	{
		if (!FilledSlots.Contains(Slot))
		{
			return Slot;
		}
	}
	// no gaps, return next index
	ensure(!FilledSlots.Contains(FilledSlots.Num() + 1));
	return FilledSlots.Num() + 1;
}

bool UGameItemContainer::IsValidSlot(int32 Slot) const
{
	return Slot >= 0 && (!GetContainerDefCDO()->bLimitSlots || Slot < GetContainerDefCDO()->SlotCount);
}

bool UGameItemContainer::IsSlotEmpty(int32 Slot) const
{
	return !ItemList.HasItemInSlot(Slot);
}

bool UGameItemContainer::CanContainItem(const UGameItem* Item) const
{
	if (!Item || !Item->GetItemDef())
	{
		return false;
	}

	for (const UGameItemContainerRule* Rule : Rules)
	{
		if (Rule && !Rule->CanContainItem(Item))
		{
			return false;
		}
	}
	return true;
}

bool UGameItemContainer::CanContainItemByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	if (!ItemDef)
	{
		return false;
	}

	for (const UGameItemContainerRule* Rule : Rules)
	{
		if (Rule && !Rule->CanContainItemByDef(ItemDef))
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
		if (!Rule)
		{
			continue;
		}

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
		if (!Rule)
		{
			continue;
		}

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

void UGameItemContainer::CreateDefaultItems(bool bForce)
{
	if (bHasDefaultItems && !bForce)
	{
		return;
	}

	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
	for (const FGameItemDefStack& DefaultItem : GetContainerDefCDO()->DefaultItems)
	{
		if (!DefaultItem.ItemDef)
		{
			continue;
		}

		UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] Creating default item: %s (x%d)"),
		       *GetNetDebugString(), *GetReadableName(), *DefaultItem.ItemDef->GetName(), DefaultItem.Count);
		ItemSubsystem->CreateItemInContainer(this, DefaultItem.ItemDef, DefaultItem.Count);
	}

	for (const UGameItemSet* ItemSet : GetContainerDefCDO()->DefaultItemSets)
	{
		if (!ItemSet)
		{
			continue;
		}

		UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] Creating default items from set: %s"),
		       *GetNetDebugString(), *GetReadableName(), *ItemSet->GetName());
		ItemSet->AddToContainer(this);
	}

	if (!GetContainerDefCDO()->DefaultDropContent.IsNull())
	{
		FGameItemDropContext Context;
		Context.TargetActor = GetOwner();

		UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] Creating default items from drop content: %s"),
		       *GetNetDebugString(), *GetReadableName(), *GetContainerDefCDO()->DefaultDropContent.ToDebugString());
		const TArray<UGameItem*> NewItems = ItemSubsystem->CreateItemsFromDropTable(this, Context, GetContainerDefCDO()->DefaultDropContent);
		AddItems(NewItems);
	}

	bHasDefaultItems = true;
}

UGameItemContainerRule* UGameItemContainer::GetRule(TSubclassOf<UGameItemContainerRule> RuleClass) const
{
	for (UGameItemContainerRule* Rule : Rules)
	{
		if (Rule && Rule->IsA(RuleClass))
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
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Rules, this);

		OnRuleAdded(NewRule);

		return NewRule;
	}
	return nullptr;
}

int32 UGameItemContainer::RemoveRule(TSubclassOf<UGameItemContainerRule> RuleClass)
{
	TArray<UGameItemContainerRule*> MatchingRules = Rules.FilterByPredicate([RuleClass](const UGameItemContainerRule* Rule)
	{
		return Rule && Rule->GetClass() == RuleClass;
	});

	int32 NumRemoved = 0;
	for (UGameItemContainerRule* Rule : MatchingRules)
	{
		Rules.Remove(Rule);
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Rules, this);

		OnRuleRemoved(Rule);

		++NumRemoved;
	}
	return NumRemoved;
}

void UGameItemContainer::OnRuleAdded(UGameItemContainerRule* Rule)
{
	check(Rule);
	OnRuleAddedEvent.Broadcast(Rule);
}

void UGameItemContainer::OnRuleRemoved(UGameItemContainerRule* Rule)
{
	check(Rule);
	OnRuleRemovedEvent.Broadcast(Rule);
}

FString UGameItemContainer::GetRulesDebugString() const
{
	FString Result;
	for (int32 Idx = 0; Idx < Rules.Num(); ++Idx)
	{
		const TObjectPtr<UGameItemContainerRule>& Rule = Rules[Idx];
		Result += Rule ? Rule->GetName() : TEXT("(null)");
		if (Idx < Rules.Num())
		{
			Result += TEXT(", ");
		}
	}
	return Result;
}

bool UGameItemContainer::IsChild() const
{
	return Algo::AnyOf(Rules, [](const UGameItemContainerRule* Rule) { return Rule && Rule->IsChild(); });
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
#if WITH_SERVER_CODE
	if (ChildContainer)
	{
		ChildContainers.AddUnique(ChildContainer);
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ChildContainers, this);
	}
#endif
}

void UGameItemContainer::UnregisterChild(UGameItemContainer* ChildContainer)
{
#if WITH_SERVER_CODE
	ChildContainers.Remove(ChildContainer);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ChildContainers, this);
#endif
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

void UGameItemContainer::TryAutoSlot(UGameItem* Item, FGameplayTagContainer ContextTags)
{
	for (const UGameItemContainerRule* Rule : Rules)
	{
		if (const UGameItemAutoSlotRule* AutoSlotRule = Cast<UGameItemAutoSlotRule>(Rule))
		{
			if (AutoSlotRule->CanAutoSlot(Item, ContextTags))
			{
				AutoSlotRule->TryAutoSlot(Item, ContextTags);
				break;
			}
		}
	}
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

void UGameItemContainer::ServerAddItem_Implementation(UGameItem* Item, int32 TargetSlot)
{
	AddItem(Item, TargetSlot);
}

void UGameItemContainer::ServerAddItems_Implementation(const TArray<UGameItem*>& Items, int32 TargetSlot)
{
	AddItems(Items, TargetSlot);
}

void UGameItemContainer::ServerRemoveItem_Implementation(UGameItem* Item)
{
	RemoveItem(Item);
}

void UGameItemContainer::ServerRemoveItems_Implementation(const TArray<UGameItem*>& Items)
{
	RemoveItems(Items);
}

void UGameItemContainer::ServerRemoveItemAt_Implementation(int32 Slot)
{
	RemoveItemAt(Slot);
}

void UGameItemContainer::ServerRemoveItemsByDef_Implementation(TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	RemoveItemsByDef(ItemDef, Count);
}

void UGameItemContainer::ServerRemoveAllItems_Implementation()
{
	RemoveAllItems();
}

void UGameItemContainer::ServerSwapItems_Implementation(int32 SlotA, int32 SlotB)
{
	SwapItems(SlotA, SlotB);
}

void UGameItemContainer::ServerStackItems_Implementation(int32 FromSlot, int32 ToSlot, bool bAllowPartial)
{
	StackItems(FromSlot, ToSlot, bAllowPartial);
}

void UGameItemContainer::ServerSetItemAt_Implementation(UGameItem* Item, int32 Slot)
{
	SetItemAt(Item, Slot);
}

void UGameItemContainer::CommitSaveData(FGameItemContainerSaveData& ContainerData, TMap<UGameItem*, FGuid>& SavedItems)
{
	// serialize all items
	bool bIsChild = IsChild();
	ContainerData.ItemList.Reset();
	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		int32 Slot = Entry.Slot;
		UGameItem* Item = Entry.Item;

		ensure(Slot != INDEX_NONE);
		ensure(IsValid(Item));

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
				UE_LOG(LogGameItems, Warning, TEXT("%s[%s] Found null item def when loading save game: %s.%d"),
					*GetNetDebugString(), *GetReadableName(), *ContainerId.ToString(), Slot);
				continue;
			}

			const TSubclassOf<UGameItemDef> ItemDef = ItemData.ItemDef.LoadSynchronous();
			if (!ItemDef)
			{
				UE_LOG(LogGameItems, Warning, TEXT("%s[%s] Failed to load item def when loading save game: %s"),
					*GetNetDebugString(), *GetReadableName(), *ItemData.ItemDef.ToString());
				continue;
			}

			UGameItem* NewItem = ItemSubsystem->CreateItem(GetItemOuter(), ItemDef);
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

EGameItemContainerNetExecutionPolicy UGameItemContainer::GetNetExecutionPolicy() const
{
	if (const UGameItemContainerDef* ContainerDefCDO = GetContainerDefCDO())
	{
		return ContainerDefCDO->NetExecutionPolicy;
	}
	return EGameItemContainerNetExecutionPolicy::ServerInitiated;
}

ENetRole UGameItemContainer::GetLocalRole() const
{
	if (GetOwner()->GetNetMode() == NM_Client)
	{
		// if owner is a player state, use controller role on clients,
		// since it will be marked as autonomous, and player state will not.
		if (const APlayerState* PlayerState = GetTypedOuter<APlayerState>())
		{
			if (const AController* Controller = PlayerState->GetOwningController())
			{
				return Controller->GetLocalRole();
			}
		}
	}
	// otherwise use direct owning actor
	return GetOwner()->GetLocalRole();
}

bool UGameItemContainer::HasAuthority() const
{
	return GetLocalRole() == ROLE_Authority;
}

bool UGameItemContainer::CanExecuteLocally() const
{
	return GetNetExecutionPolicy() == EGameItemContainerNetExecutionPolicy::LocalPredicted && GetLocalRole() != ROLE_SimulatedProxy;
}

void UGameItemContainer::OnItemAdded(UGameItem* Item, int32 Slot)
{
	check(Item);
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] [%hs] [Slot %d] %s"),
	       *GetNetDebugString(), *GetReadableName(), __func__, Slot, *Item->GetDebugString());

	Item->Containers.AddUnique(this);
	OnItemAddedEvent.Broadcast(Item);
	Item->OnSlottedEvent.Broadcast(this, Slot, INDEX_NONE);
}

void UGameItemContainer::OnItemRemoved(UGameItem* Item, int32 Slot)
{
	check(Item);
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] [%hs] [Slot %d] %s"),
	       *GetNetDebugString(), *GetReadableName(), __func__, Slot, *Item->GetDebugString());

	Item->Containers.Remove(this);
	OnItemRemovedEvent.Broadcast(Item);
	Item->OnUnslottedEvent.Broadcast(this, Slot);
}

void UGameItemContainer::OnPostReplicatedChanges(const TArray<FGameItemList::FChange>& Changes)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] [%hs] Received %d changes..."),
		   *GetNetDebugString(), *GetReadableName(), __func__, Changes.Num());

	FScopedSlotChanges SlotChangeScope(this);

	for (int32 Idx = 0; Idx < Changes.Num(); ++Idx)
	{
		const FGameItemList::FChange& Change = Changes[Idx];

		UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] [%hs]   %d: %s"),
		       *GetNetDebugString(), *GetReadableName(), __func__, Idx, *Change.GetDebugString());

		if (!Change.Item)
		{
			// ignore updates before item has replicated
			continue;
		}

		if (Change.bIsRemoved)
		{
			// should match RemoveItemAt
			const bool bShouldCollapse = !GetContainerDefCDO()->bLimitSlots;
			OnItemRemoved(Change.Item, Change.Slot);
			OnSlotRangeChanged(Change.Slot, bShouldCollapse ? GetNumSlots() - 1 : Change.Slot);
		}
		else
		{
			// new item
			if (!Change.Item->Containers.Contains(this))
			{
				OnItemAdded(Change.Item, Change.Slot);
				OnSlotChanged(Change.Slot);
			}

			// changed slots
			if (Change.LastKnownSlot != Change.Slot && Change.LastKnownSlot != INDEX_NONE)
			{
				OnSlotChanged(Change.Slot);
				OnSlotChanged(Change.LastKnownSlot);
			}
		}
	}
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
			: Start(Slot)
			, End(Slot)
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
			UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] OnItemSlotChanged [Slot %d]"),
			       *GetNetDebugString(), *GetReadableName(), Range.Start);
			OnItemSlotChangedEvent.Broadcast(Range.Start);
		}
		else
		{
			UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] OnItemSlotsChanged [Slot %d..%d]"),
			       *GetNetDebugString(), *GetReadableName(), Range.Start, Range.End);
			OnItemSlotsChangedEvent.Broadcast(Range.Start, Range.End);
		}
	};

	// aggregate into adjacent ranges
	TArray<int32> SlotsArray = PendingChangedSlots.Array();
	PendingChangedSlots.Reset();

	FSlotRange CurrentRange;
	for (int32 Idx = 0; Idx < SlotsArray.Num(); ++Idx)
	{
		int32 Slot = SlotsArray[Idx];
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

		if (Idx == SlotsArray.Num() - 1)
		{
			BroadcastRange(CurrentRange);
		}
	}

	if (NumSlotsPreChange != INDEX_NONE)
	{
		const int32 NewNumSlots = GetNumSlots();
		if (NumSlotsPreChange != NewNumSlots)
		{
			UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] OnNumSlotsChanged %d -> %d"),
			       *GetNetDebugString(), *GetReadableName(), NumSlotsPreChange, NewNumSlots);
			OnNumSlotsChangedEvent.Broadcast(NewNumSlots, NumSlotsPreChange);
		}
	}
}

void UGameItemContainer::OnRep_Rules(const TArray<UGameItemContainerRule*>& PreviousRules)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] [%hs] Rules: %s"),
	       *GetNetDebugString(), *GetReadableName(), __func__, *GetRulesDebugString());

	// find rules that got removed
	for (UGameItemContainerRule* PreviousRule : PreviousRules)
	{
		if (IsValid(PreviousRule))
		{
			if (!Rules.Contains(PreviousRule))
			{
				OnRuleRemoved(PreviousRule);
			}
		}
	}

	// find rules that got added
	for (UGameItemContainerRule* NewRule : Rules)
	{
		if (IsValid(NewRule))
		{
			if (!PreviousRules.Contains(NewRule))
			{
				OnRuleAdded(NewRule);
			}
		}
	}
}

void UGameItemContainer::OnSlotChanged(int32 Slot)
{
	PendingChangedSlots.Add(Slot);
}

void UGameItemContainer::OnSlotsChanged(const TArray<int32>& Slots)
{
	for (const int32& Slot : Slots)
	{
		PendingChangedSlots.Add(Slot);
	}
}

void UGameItemContainer::OnSlotRangeChanged(int32 StartSlot, int32 EndSlot)
{
	for (int32 Slot = StartSlot; Slot <= EndSlot; ++Slot)
	{
		PendingChangedSlots.Add(Slot);
	}
}

FString UGameItemContainer::GetReadableName() const
{
	if (const UActorComponent* CompOuter = Cast<UActorComponent>(GetOuter()))
	{
		return FString::Printf(TEXT("%s/%s"), *CompOuter->GetReadableName(), *ContainerId.ToString());
	}
	if (const UObject* ObjectOuter = Cast<AActor>(GetOuter()))
	{
		return FString::Printf(TEXT("%s/%s"), *ObjectOuter->GetName(), *ContainerId.ToString());
	}
	// no outer
	return ContainerId.ToString();
}

void UGameItemContainer::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) const
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::White);
	const FString CountStr = GetContainerDefCDO()->bLimitSlots
		? FString::Printf(TEXT("%d/%d"), GetNumItems(), GetContainerDefCDO()->SlotCount)
		: FString::FromInt(GetNumItems());
	DisplayDebugManager.DrawString(FString::Printf(TEXT("%s (%s items)"), *ContainerId.ToString(), *CountStr));

	for (const FGameItemListEntry& Entry : ItemList.GetEntries())
	{
		DisplayDebugManager.DrawString(FString::Printf(TEXT("    [%d] %s"), Entry.Slot, *Entry.GetDebugString()));
	}
}
