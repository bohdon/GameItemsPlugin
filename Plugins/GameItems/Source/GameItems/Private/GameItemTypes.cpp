// Copyright Bohdon Sayre, All Rights Reserved.

#include "GameItemTypes.h"

#include "GameItem.h"
#include "GameItemDef.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemTypes)


// FGameItemTagStack
// -----------------

FString FGameItemTagStack::ToDebugString() const
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
		StackStrings.Add(Stack.ToDebugString());
	}
	return FString::Join(StackStrings, TEXT(", "));
}


// FGameItemListEntry
// ------------------

FString FGameItemListEntry::ToDebugString() const
{
	return Item ? Item->ToDebugString() : TEXT("(none)");
}


// FGameItemList
// -------------

void FGameItemList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Idx : RemovedIndices)
	{
		// FGameItemListEntry& Entry = Entries[Idx];
		// OnListChangedEvent.Broadcast(Entry, 0, Entry.Count);
		// Entry.LastKnownCount = 0;
	}
}

void FGameItemList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		// FGameItemListEntry& Entry = Entries[Idx];
		// OnListChangedEvent.Broadcast(Entry, Entry.Count, 0);
		// Entry.LastKnownCount = Entry.Count;
	}
}

void FGameItemList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (const int32 Idx : ChangedIndices)
	{
		// FGameItemListEntry& Entry = Entries[Idx];
		// OnListChangedEvent.Broadcast(Entry, Entry.LastKnownCount, Entry.Count);
		// Entry.LastKnownCount = Entry.Count;
	}
}

void FGameItemList::AddEntry(UGameItem* Item)
{
	FGameItemListEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Item = Item;

	MarkItemDirty(NewEntry);
}

void FGameItemList::RemoveEntry(UGameItem* Item)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FGameItemListEntry& Entry = *EntryIt;
		if (Entry.Item == Item)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

void FGameItemList::GetAllItems(TArray<UGameItem*>& OutItems) const
{
	OutItems.Reset(Entries.Num());
	for (const FGameItemListEntry& Entry : Entries)
	{
		OutItems.Add(Entry.Item);
	}
}
