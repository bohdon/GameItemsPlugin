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

void FGameEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (const int32 Idx : RemovedIndices)
	{
		const FGameEquipmentListEntry& Entry = Entries[Idx];
		if (Entry.Equipment != nullptr)
		{
			Entry.Equipment->OnUnequipped();
		}
	}
}

void FGameEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		const FGameEquipmentListEntry& Entry = Entries[Idx];
		if (Entry.Equipment != nullptr)
		{
			Entry.Equipment->OnEquipped();
		}
	}
}

void FGameEquipmentList::AddEntry(UGameEquipment* Equipment)
{
	FGameEquipmentListEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Equipment = Equipment;

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
