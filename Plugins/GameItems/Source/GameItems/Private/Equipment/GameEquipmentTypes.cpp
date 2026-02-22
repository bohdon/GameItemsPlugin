// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameEquipmentTypes.h"

#include "Equipment/GameEquipment.h"


// FGameEquipmentSpec
// ------------------

FGameEquipmentSpec::FGameEquipmentSpec(const TSubclassOf<UGameEquipmentDef>& InEquipmentDef, const TArray<FGameItemTagStack>& InTagStats)
	: EquipmentDef(InEquipmentDef)
	, TagStats(InTagStats)
{
	TagStatsMap.Reserve(TagStats.Num());
	for (const FGameItemTagStack& Stack : TagStats)
	{
		TagStatsMap.Add(Stack.Tag, Stack.Count);
	}
}

FGameEquipmentSpec::FGameEquipmentSpec(
	const TSubclassOf<UGameEquipmentDef>& InEquipmentDef,
	const TArray<FGameItemTagStack>& InTagStats,
	const FGameplayTagContainer& InContextTags)
	: EquipmentDef(InEquipmentDef)
	, ContextTags(InContextTags)
	, TagStats(InTagStats)
{
	TagStatsMap.Reserve(TagStats.Num());
	for (const FGameItemTagStack& Stack : TagStats)
	{
		TagStatsMap.Add(Stack.Tag, Stack.Count);
	}
}

void FGameEquipmentSpec::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading())
	{
		TagStatsMap.Reset();
		for (const FGameItemTagStack& Stack : TagStats)
		{
			TagStatsMap.Add(Stack.Tag, Stack.Count);
		}
	}
}


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
