// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameEquipmentTypes.h"

#include "Equipment/GameEquipment.h"


// FGameEquipmentListEntry
// -----------------------

FString FGameEquipmentListEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s"), *GetNameSafe(Equipment));
}


// FGameEquipmentList
// ------------------

void FGameEquipmentList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	for (const int32 Idx : RemovedIndices)
	{
		OnPreReplicatedRemoveEvent.Broadcast(Entries[Idx]);
	}
}

void FGameEquipmentList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		OnPostReplicatedAddEvent.Broadcast(Entries[Idx]);
	}
}

void FGameEquipmentList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (const int32 Idx : ChangedIndices)
	{
		OnPostReplicatedChangeEvent.Broadcast(Entries[Idx]);
	}
}

void FGameEquipmentList::AddEntry(UGameEquipment* Equipment)
{
FGameEquipmentListEntry& NewEntry = Entries.Emplace_GetRef(Equipment);
	MarkItemDirty(NewEntry);
}

void FGameEquipmentList::RemoveEntry(UGameEquipment* Equipment)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FGameEquipmentListEntry& Entry = *EntryIt;
		if (IsValid(Entry.Equipment) && Entry.Equipment == Equipment)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}
