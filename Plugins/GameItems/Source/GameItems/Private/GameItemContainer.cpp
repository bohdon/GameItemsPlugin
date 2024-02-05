// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainer.h"

#include "GameItem.h"
#include "GameItemContainerDef.h"
#include "GameItemContainerStockRule.h"
#include "GameItemDef.h"
#include "GameItemSet.h"
#include "GameItemsModule.h"
#include "GameItemSubsystem.h"
#include "Engine/ActorChannel.h"
#include "Engine/Canvas.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainer)


UGameItemContainer::UGameItemContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ItemList.OnListChangedEvent.AddUObject(this, &UGameItemContainer::OnListChanged);
}

void UGameItemContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemList);
}

UGameItem* UGameItemContainer::CreateItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count) const
{
	check(ItemDef);

	AActor* OwningActor = GetOwner();
	check(OwningActor->HasAuthority());

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameItemSubsystem* ItemSubsystem = UGameInstance::GetSubsystem<UGameItemSubsystem>(GetWorld()->GetGameInstance());
	return ItemSubsystem->CreateGameItem(OwningActor, ItemDef, Count);
}

UGameItem* UGameItemContainer::DuplicateItem(UGameItem* Item) const
{
	check(Item != nullptr);

	AActor* OwningActor = GetOwner();
	check(OwningActor->HasAuthority());

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameItemSubsystem* ItemSubsystem = World->GetGameInstance()->GetSubsystem<UGameItemSubsystem>();

	return ItemSubsystem->DuplicateGameItem(OwningActor, Item);
}

const UGameItemContainerDef* UGameItemContainer::GetContainerDefCDO() const
{
	return GetDefault<UGameItemContainerDef>(ContainerDef);
}

bool UGameItemContainer::CanAddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	// TODO: add stack count limits, etc
	return true;
}

UGameItem* UGameItemContainer::AddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	// TODO: move to subsystem, all item creation should go thru there, e.g. create-and-add-to-container
	if (!ItemDef)
	{
		return nullptr;
	}

	UGameItem* NewItem = CreateItem(ItemDef, Count);
	if (!NewItem)
	{
		return nullptr;
	}

	AddItem(NewItem);

	return NewItem;
}

TArray<UGameItem*> UGameItemContainer::AddItem(UGameItem* Item, int32 TargetSlot)
{
	if (!Item)
	{
		return TArray<UGameItem*>();
	}

	// the quantity of the item to add
	int32 DeltaCount = Item->GetCount();

	// calculate and clamp the total quantity of the item that can be added across multiple stacks.
	// if loss of the item's count is not desired, those checks should have been performed before this point
	const int32 OldTotalCount = GetTotalMatchingItemCount(Item);
	const int32 MaxCount = GetItemMaxCount(Item);
	// TODO: perform more accurate calculation, check available slots, remaining space in existing stacks, etc
	const int32 MaxDeltaCount = FMath::Max(MaxCount - OldTotalCount, 0);
	DeltaCount = FMath::Min(DeltaCount, MaxDeltaCount);

	if (DeltaCount == 0)
	{
		UE_LOG(LogGameItems, Verbose, TEXT("%s: Cant add item, max count reached: %s"),
		       *GetNameSafe(GetOwner()), *Item->ToDebugString());
		return TArray<UGameItem*>();
	}

	if (DeltaCount < Item->GetCount())
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s: Adding %s, but %d will be lost due to capacity, check GetItemMaxCount before adding to prevent this."),
		       *GetNameSafe(GetOwner()), *Item->ToDebugString(), Item->GetCount() - DeltaCount);
	}

	// TODO: ensure item is removed from any existing container

	// repeatedly add the item, splitting and stacking as necessary
	TArray<UGameItem*> Result;
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

	while (RemainingCountToAdd > 0)
	{
		const int32 NumEmptySlots = GetNumEmptySlots();
		if (NumEmptySlots == 0 && MatchingItemsWithSpace.IsEmpty())
		{
			// out of space
			UE_LOG(LogGameItems, Warning, TEXT("%s: Adding %s, but %d was lost due to limited slot capacity."),
			       *GetNameSafe(GetOwner()), *Item->ToDebugString(), RemainingCountToAdd);
			break;
		}

		// if no specific slot was given, start at 0
		if (NextTargetSlot < 0)
		{
			NextTargetSlot = 0;
		}

		// stack with existing items when auto stacking is enabled, or when there are no empty slots left
		const bool bCanStackWithExisting = GetContainerDefCDO()->bAutoStack || NumEmptySlots == 0;

		// attempt to add to next target slot
		UGameItem* ExistingItem = GetItemAt(NextTargetSlot);
		if (ExistingItem && bCanStackWithExisting && Item->IsMatching(ExistingItem) && ExistingItem->GetCount() < StackMaxCount)
		{
			// found matching item with space, add to it
			const int32 DeltaToAdd = FMath::Min(RemainingCountToAdd, StackMaxCount - ExistingItem->GetCount());

			ExistingItem->SetCount(ExistingItem->GetCount() + DeltaToAdd);
			RemainingCountToAdd -= DeltaToAdd;

			if (MatchingItemsWithSpace.Contains(ExistingItem))
			{
				MatchingItemsWithSpace.Remove(ExistingItem);
			}

			Result.Add(ExistingItem);
		}
		else if (!ExistingItem)
		{
			// found empty slot, add to it
			const int32 DeltaToAdd = FMath::Min(RemainingCountToAdd, StackMaxCount);

			// use the given item first, but if its already been added, duplicate and add a new item.
			// don't worry about the given items original count, RemainingCountToAdd is all that matters.
			UGameItem* NewItem = Item;
			if (Result.Contains(Item))
			{
				NewItem = DuplicateItem(Item);
			}
			NewItem->SetCount(DeltaToAdd);
			RemainingCountToAdd -= DeltaToAdd;

			ItemList.AddEntryAt(Item, NextTargetSlot);
			OnItemAdded(Item);

			Result.Add(NewItem);
		}

		// increment target slot and continue
		++NextTargetSlot;
	}

	return Result;
}

void UGameItemContainer::RemoveItem(UGameItem* Item)
{
	if (!Item)
	{
		return;
	}

	ItemList.RemoveEntry(Item);
	OnItemRemoved(Item);
}

UGameItem* UGameItemContainer::RemoveItemAt(int32 Slot)
{
	UGameItem* RemovedItem = ItemList.RemoveEntryAt(Slot);
	OnItemRemoved(RemovedItem);

	return RemovedItem;
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

int32 UGameItemContainer::GetItemSlot(UGameItem* Item) const
{
	return ItemList.Entries.IndexOfByPredicate([Item](const FGameItemListEntry& Entry)
	{
		return Entry.GetItem() == Item;
	});
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

int32 UGameItemContainer::GetNumItems() const
{
	return ItemList.Entries.Num();
}

int32 UGameItemContainer::GetNumSlots() const
{
	return GetContainerDefCDO()->bLimitSlots ? GetContainerDefCDO()->SlotCount : GetNumItems();
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
		return -1;
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

bool UGameItemContainer::IsSlotEmpty(int32 Slot) const
{
	return !ItemList.Entries.IsValidIndex(Slot) || ItemList.Entries[Slot].GetItem() == nullptr;
}

int32 UGameItemContainer::GetItemMaxCount(const UGameItem* Item) const
{
	if (!Item)
	{
		return 0;
	}

	const UGameItemDef* ItemDefCDO = Item->GetItemDefCDO();
	if (!ItemDefCDO)
	{
		return 0;
	}

	// find the smallest stack limit as defined by stock rules
	int32 Result = MAX_int32;

	if (ItemDefCDO->StockRules.bLimitCount)
	{
		Result = ItemDefCDO->StockRules.MaxCount;
	}

	for (const UGameItemContainerStockRule* StockRule : GetContainerDefCDO()->StockRules)
	{
		const int32 RuleMaxCount = StockRule->GetItemMaxCount(this, Item);
		if (RuleMaxCount >= 0)
		{
			Result = FMath::Min(Result, RuleMaxCount);
		}
	}

	return Result;
}

int32 UGameItemContainer::GetItemStackMaxCount(const UGameItem* Item) const
{
	if (!Item)
	{
		return 0;
	}

	const UGameItemDef* ItemDefCDO = Item->GetItemDefCDO();
	if (!ItemDefCDO)
	{
		return 0;
	}

	// find the smallest stack limit as defined by stock rules
	int32 Result = MAX_int32;

	if (ItemDefCDO->StockRules.bLimitStackCount)
	{
		Result = ItemDefCDO->StockRules.StackMaxCount;
	}

	for (const UGameItemContainerStockRule* StockRule : GetContainerDefCDO()->StockRules)
	{
		const int32 RuleMaxCount = StockRule->GetItemStackMaxCount(this, Item);
		if (RuleMaxCount >= 0)
		{
			Result = FMath::Min(Result, RuleMaxCount);
		}
	}

	return Result;
}

void UGameItemContainer::AddDefaultItems(bool bForce)
{
	if (bHasDefaultItems && !bForce)
	{
		return;
	}

	for (const FGameItemDefStack& DefaultItem : GetContainerDefCDO()->DefaultItems)
	{
		AddNewItem(DefaultItem.ItemDef, DefaultItem.Count);
	}

	for (const UGameItemSet* ItemSet : GetContainerDefCDO()->DefaultItemSets)
	{
		if (!ItemSet)
		{
			continue;
		}

		ItemSet->AddToContainer(this);
	}

	bHasDefaultItems = true;
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

void UGameItemContainer::OnItemAdded(UGameItem* Item)
{
	OnItemAddedEvent.Broadcast(Item);

	// if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Item)
	// {
	// 	AddReplicatedSubObject(Item);
	// }
}

void UGameItemContainer::OnItemRemoved(UGameItem* Item)
{
	OnItemRemovedEvent.Broadcast(Item);

	// if (IsUsingRegisteredSubObjectList() && Item)
	// {
	// 	RemoveReplicatedSubObject(Item);
	// }
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
