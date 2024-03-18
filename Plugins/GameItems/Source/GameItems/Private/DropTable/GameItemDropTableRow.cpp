// Copyright Bohdon Sayre, All Rights Reserved.


#include "DropTable/GameItemDropTableRow.h"

#include "DropTable/GameItemDropContent.h"


void FGameItemDropTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	// notify content structs when data has changed
	for (TInstancedStruct<FGameItemDropContent>& Entry : Content)
	{
		if (FGameItemDropContent* EntryContent = Entry.GetMutablePtr<FGameItemDropContent>())
		{
			EntryContent->OnDataChanged();
		}
	}
}
