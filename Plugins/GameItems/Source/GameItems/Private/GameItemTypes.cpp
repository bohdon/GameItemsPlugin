// Copyright Bohdon Sayre, All Rights Reserved.

#include "GameItemTypes.h"

#include "GameItem.h"
#include "GameItemDef.h"
#include "GameItemsModule.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemTypes)


// FGameItemCountLimit
// -------------------

int32 FGameItemCountLimit::GetMaxCount(int32 Default) const
{
	return bLimitCount ? MaxCount : Default;
}


// FGameItemTagStack
// -----------------

FString FGameItemTagStack::GetDebugString() const
{
	return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), Count);
}


// FGameItemTagStackContainer
// --------------------------

void FGameItemTagStackContainer::AddStack(FGameplayTag Tag, int32 DeltaCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddStack"), ELogVerbosity::Warning);
		return;
	}

	if (DeltaCount <= 0)
	{
		// nothing to add
		return;
	}

	for (FGameItemTagStack& Stack : Stacks)
	{
		if (Stack.Tag == Tag)
		{
			Stack.Count += DeltaCount;
			StackCountMap[Tag] = Stack.Count;
			MarkItemDirty(Stack);
			return;
		}
	}

	FGameItemTagStack& NewStack = Stacks.Emplace_GetRef(Tag, DeltaCount);
	MarkItemDirty(NewStack);
	StackCountMap.Add(Tag, DeltaCount);
}

void FGameItemTagStackContainer::RemoveStack(FGameplayTag Tag, int32 DeltaCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStack"), ELogVerbosity::Warning);
		return;
	}

	if (DeltaCount <= 0)
	{
		return;
	}

	for (auto It = Stacks.CreateIterator(); It; ++It)
	{
		FGameItemTagStack& Stack = *It;
		if (Stack.Tag == Tag)
		{
			if (Stack.Count <= DeltaCount)
			{
				// remove the tag entirely
				It.RemoveCurrent();
				StackCountMap.Remove(Tag);
				MarkArrayDirty();
			}
			else
			{
				// decrease the stack count
				Stack.Count -= DeltaCount;
				StackCountMap[Tag] = Stack.Count;
				MarkItemDirty(Stack);
			}
			return;
		}
	}
}

void FGameItemTagStackContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Idx : RemovedIndices)
	{
		const FGameplayTag Tag = Stacks[Idx].Tag;
		StackCountMap.Remove(Tag);
	}
}

void FGameItemTagStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		const FGameItemTagStack& Stack = Stacks[Idx];
		StackCountMap.Add(Stack.Tag, Stack.Count);
	}
}

void FGameItemTagStackContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Idx : ChangedIndices)
	{
		const FGameItemTagStack& Stack = Stacks[Idx];
		StackCountMap[Stack.Tag] = Stack.Count;
	}
}

void FGameItemTagStackContainer::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading())
	{
		// update StackCountMap after load
		StackCountMap.Reset();
		for (const FGameItemTagStack& Stack : Stacks)
		{
			StackCountMap.Add(Stack.Tag, Stack.Count);
		}
	}
}

FString FGameItemTagStackContainer::ToDebugString() const
{
	TArray<FString> StackStrings;
	for (const FGameItemTagStack& Stack : Stacks)
	{
		StackStrings.Add(Stack.GetDebugString());
	}
	return FString::Join(StackStrings, TEXT(", "));
}


// FGameItemListEntry
// ------------------

FString FGameItemListEntry::GetDebugString() const
{
	return FString::Printf(TEXT("[Slot %d]: %s"), Slot, Item ? *Item->ToDebugString() : TEXT("(invalid)"));
}


// FGameItemList
// -------------

void FGameItemList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	// called when any item is removed from a slot,
	// (even if replaced by another item, since that will be a new entry)
	for (const int32 Idx : RemovedIndices)
	{
		OnPreReplicatedRemoveEvent.Broadcast(Entries[Idx]);
	}
}

void FGameItemList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		// TODO: Entry.Item is always null here (before UpdateUnmappedObjects is called), but should be valid 
		OnPostReplicatedAddEvent.Broadcast(Entries[Idx]);
	}
}

void FGameItemList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	// TODO: currently called when Entry.Item goes from null -> valid, which should ideally happen before PostReplicatedAdd
	for (const int32 Idx : ChangedIndices)
	{
		OnPostReplicatedChangeEvent.Broadcast(Entries[Idx]);
	}
}

void FGameItemList::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading())
	{
		// TODO: cache slot -> entry lookup map for fast access
	}
}

void FGameItemList::AddEntryForSlot(UGameItem* Item, int32 Slot)
{
	check(Item != nullptr);
	check(Slot >= 0);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// ensure slot is new
	if (!ensureAlwaysMsgf(!HasItemInSlot(Slot), TEXT("Entry already exists for slot: %d"), Slot))
	{
		return;
	}
#endif

	FGameItemListEntry& NewEntry = Entries.Emplace_GetRef(Item, Slot);
	MarkItemDirty(NewEntry);
}

void FGameItemList::RemoveEntry(UGameItem* Item)
{
	check(Item != nullptr);

	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FGameItemListEntry& Entry = *EntryIt;
		if (Entry.Item == Item)
		{
			// TODO: try RemoveCurrentSwap, see if it's better/works
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

UGameItem* FGameItemList::RemoveEntryForSlot(int32 Slot)
{
	UGameItem* RemovedItem = nullptr;
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		if (EntryIt->Slot == Slot)
		{
			RemovedItem = EntryIt->Item;
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
			break;
		}
	}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	ensureAlwaysMsgf(!HasItemInSlot(Slot), TEXT("Multiple entries found for slot: %d"), Slot);
#endif
	return RemovedItem;
}

UGameItem* FGameItemList::GetItemInSlot(int32 Slot) const
{
	// TODO: cache
	for (const FGameItemListEntry& Entry : Entries)
	{
		if (Entry.Slot == Slot)
		{
			return Entry.Item;
		}
	}
	return nullptr;
}

bool FGameItemList::HasItemInSlot(int32 Slot) const
{
	// TODO: cache
	for (const FGameItemListEntry& Entry : Entries)
	{
		if (Entry.Slot == Slot && ensureAlways(Entry.Item))
		{
			return true;
		}
	}
	return false;
}

void FGameItemList::Reset()
{
	Entries.Reset();
	MarkArrayDirty();
}

void FGameItemList::SwapEntries(int32 SlotA, int32 SlotB)
{
	check(SlotA >= 0);
	check(SlotB >= 0);

	const int32 MaxIndex = FMath::Max(SlotA, SlotB);
	if (MaxIndex >= Entries.Num() - 1)
	{
		Entries.SetNum(MaxIndex + 1);
	}

	Entries.Swap(SlotA, SlotB);
	MarkArrayDirty();
}

void FGameItemList::GetAllItems(TMap<int32, UGameItem*>& OutItems) const
{
	OutItems.Reset();
	OutItems.Reserve(Entries.Num());
	for (const FGameItemListEntry& Entry : Entries)
	{
		OutItems.Add(Entry.Slot, Entry.Item);
	}
}

void FGameItemList::GetAllSlots(TArray<int32>& OutSlots) const
{
	OutSlots.Reset();
	OutSlots.Reserve(Entries.Num());
	for (const FGameItemListEntry& Entry : Entries)
	{
		ensureAlways(!OutSlots.Contains(Entry.Slot));
		OutSlots.Add(Entry.Slot);
	}
	OutSlots.Sort();
}


// FGameItemSaveData
// -----------------

FGameItemSaveData::FGameItemSaveData()
{
}

FGameItemSaveData::FGameItemSaveData(UGameItem* InItem)
	: FGameItemSaveData()
{
	if (!InItem)
	{
		return;
	}

	ItemDef = InItem->GetItemDef();

	FMemoryWriter MemWriter(ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	InItem->Serialize(Ar);

	// create a new guid for this save data
	Guid = FGuid::NewGuid();
}

FGameItemSaveData::FGameItemSaveData(const FGuid& InGuid)
	: Guid(InGuid)
{
}
