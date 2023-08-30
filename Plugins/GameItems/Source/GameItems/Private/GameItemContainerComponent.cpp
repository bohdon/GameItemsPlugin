// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerComponent.h"

#include "GameItem.h"
#include "GameItemContainerStockRule.h"
#include "GameItemDef.h"
#include "GameItemSet.h"
#include "GameItemsModule.h"
#include "GameItemSubsystem.h"
#include "Engine/ActorChannel.h"
#include "Engine/Canvas.h"
#include "Net/UnrealNetwork.h"


UGameItemContainerComponent::UGameItemContainerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  bLimitSlots(false),
	  SlotCount(10),
	  bAutoStack(true)
{
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);

	ItemList.OnListChangedEvent.AddUObject(this, &UGameItemContainerComponent::OnListChanged);
}

void UGameItemContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemList);
}

void UGameItemContainerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	const UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		AddDefaultItems();
	}
}

UGameItem* UGameItemContainerComponent::CreateItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count) const
{
	check(ItemDef != nullptr);

	AActor* OwningActor = GetOwner();
	check(OwningActor->HasAuthority());

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameItemSubsystem* ItemSubsystem = World->GetGameInstance()->GetSubsystem<UGameItemSubsystem>();

	return ItemSubsystem->CreateGameItem(OwningActor, ItemDef, Count);
}

UGameItem* UGameItemContainerComponent::DuplicateItem(UGameItem* Item) const
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

bool UGameItemContainerComponent::CanAddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	// TODO: add stack count limits, etc
	return true;
}

UGameItem* UGameItemContainerComponent::AddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
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

TArray<UGameItem*> UGameItemContainerComponent::AddItem(UGameItem* Item, int32 TargetSlot)
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
	if (bAutoStack || bLimitSlots)
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
		const bool bCanStackWithExisting = bAutoStack || NumEmptySlots == 0;

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

void UGameItemContainerComponent::RemoveItem(UGameItem* Item)
{
	if (!Item)
	{
		return;
	}

	ItemList.RemoveEntry(Item);
	OnItemRemoved(Item);
}

UGameItem* UGameItemContainerComponent::RemoveItemAt(int32 Slot)
{
	UGameItem* RemovedItem = ItemList.RemoveEntryAt(Slot);
	OnItemRemoved(RemovedItem);

	return RemovedItem;
}

TArray<UGameItem*> UGameItemContainerComponent::GetAllItems() const
{
	TArray<UGameItem*> Result;
	ItemList.GetAllItems(Result);
	return Result;
}

UGameItem* UGameItemContainerComponent::GetItemAt(int32 Slot) const
{
	return ItemList.Entries.IsValidIndex(Slot) ? ItemList.Entries[Slot].Item : nullptr;
}

UGameItem* UGameItemContainerComponent::FindFirstItemByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item) && Entry.Item->GetItemDef() == ItemDef)
		{
			return Entry.Item;
		}
	}
	return nullptr;
}

UGameItem* UGameItemContainerComponent::FindFirstMatchingItem(const UGameItem* Item) const
{
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item) && Entry.Item->IsMatching(Item))
		{
			return Entry.Item;
		}
	}
	return nullptr;
}

TArray<UGameItem*> UGameItemContainerComponent::GetAllMatchingItems(const UGameItem* Item) const
{
	TArray<UGameItem*> Result;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item) && Entry.Item->IsMatching(Item))
		{
			Result.Add(Entry.Item);
		}
	}
	return Result;
}

int32 UGameItemContainerComponent::GetItemSlot(UGameItem* Item) const
{
	return ItemList.Entries.IndexOfByPredicate([Item](const FGameItemListEntry& Entry)
	{
		return Entry.Item == Item;
	});
}

int32 UGameItemContainerComponent::GetTotalItemCountByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	int32 Total = 0;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item) && Entry.Item->GetItemDef() == ItemDef)
		{
			Total += Entry.Item->GetCount();
		}
	}
	return Total;
}

int32 UGameItemContainerComponent::GetTotalMatchingItemCount(const UGameItem* Item) const
{
	int32 Total = 0;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item) && Entry.Item->IsMatching(Item))
		{
			Total += Entry.Item->GetCount();
		}
	}
	return Total;
}

int32 UGameItemContainerComponent::GetTotalItemCount() const
{
	int32 Total = 0;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item))
		{
			Total += Entry.Item->GetCount();
		}
	}
	return Total;
}

int32 UGameItemContainerComponent::GetNumItems() const
{
	return ItemList.Entries.Num();
}

int32 UGameItemContainerComponent::GetNumEmptySlots() const
{
	// always return zero when no space is left, even if container was overfilled somehow
	return bLimitSlots ? FMath::Max(SlotCount - GetNumItems(), 0) : INDEX_NONE;
}

int32 UGameItemContainerComponent::GetNextEmptySlot() const
{
	if (bLimitSlots)
	{
		// iterate over limited slot count
		for (int32 Idx = 0; Idx < SlotCount; ++Idx)
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

bool UGameItemContainerComponent::IsSlotEmpty(int32 Slot) const
{
	return !ItemList.Entries.IsValidIndex(Slot) || ItemList.Entries[Slot].Item == nullptr;
}

int32 UGameItemContainerComponent::GetItemMaxCount(const UGameItem* Item) const
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

	for (const UGameItemContainerStockRule* StockRule : StockRules)
	{
		const int32 RuleMaxCount = StockRule->GetItemMaxCount(this, Item);
		if (RuleMaxCount >= 0)
		{
			Result = FMath::Min(Result, RuleMaxCount);
		}
	}

	return Result;
}

int32 UGameItemContainerComponent::GetItemStackMaxCount(const UGameItem* Item) const
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

	for (const UGameItemContainerStockRule* StockRule : StockRules)
	{
		const int32 RuleMaxCount = StockRule->GetItemStackMaxCount(this, Item);
		if (RuleMaxCount >= 0)
		{
			Result = FMath::Min(Result, RuleMaxCount);
		}
	}

	return Result;
}

void UGameItemContainerComponent::AddDefaultItems(bool bForce)
{
	if (bHasDefaultItems && !bForce)
	{
		return;
	}

	for (const FGameItemDefStack& DefaultItem : DefaultItems)
	{
		AddNewItem(DefaultItem.ItemDef, DefaultItem.Count);
	}

	for (const UGameItemSet* ItemSet : DefaultItemSets)
	{
		if (!ItemSet)
		{
			continue;
		}

		for (const FGameItemDefStack& DefaultItem : ItemSet->Items)
		{
			AddNewItem(DefaultItem.ItemDef, DefaultItem.Count);
		}
	}

	bHasDefaultItems = true;
}

void UGameItemContainerComponent::OnItemAdded(UGameItem* Item)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Item)
	{
		AddReplicatedSubObject(Item);
	}
}

void UGameItemContainerComponent::OnItemRemoved(UGameItem* Item)
{
	if (IsUsingRegisteredSubObjectList() && Item)
	{
		RemoveReplicatedSubObject(Item);
	}
}

void UGameItemContainerComponent::OnListChanged(FGameItemListEntry& Entry, int32 NewCount, int32 OldCount)
{
	// re-broadcast the change from this component
	// TODO
}

void UGameItemContainerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// register any existing items
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FGameItemListEntry& Entry : ItemList.Entries)
		{
			if (IsValid(Entry.Item))
			{
				AddReplicatedSubObject(Entry.Item);
			}
		}
	}
}

bool UGameItemContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bDidWrite = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// replicate all item instances in this container
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item))
		{
			bDidWrite |= Channel->ReplicateSubobject(Entry.Item, *Bunch, *RepFlags);
		}
	}

	return bDidWrite;
}


void UGameItemContainerComponent::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) const
{
	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetDrawColor(FColor::White);
	const FString CountStr = bLimitSlots ? FString::Printf(TEXT("%d/%d"), GetNumItems(), SlotCount) : FString::FromInt(GetNumItems());
	DisplayDebugManager.DrawString(FString::Printf(TEXT("%s.%s (%s items)"), *GetOwner()->GetName(), *GetName(), *CountStr));

	for (int32 Idx = 0; Idx < ItemList.Entries.Num(); ++Idx)
	{
		const FGameItemListEntry& Entry = ItemList.Entries[Idx];
		DisplayDebugManager.DrawString(FString::Printf(TEXT("    [%d] %s"), Idx, *Entry.ToDebugString()));
	}
}
