// Copyright Bohdon Sayre, All Rights Reserved.


#include "DropTable/GameItemDropTableRow.h"

#include "DropTable/GameItemDropContent.h"


void FGameItemDropTableRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	// notify content when data has changed
	if (FGameItemDropContent* ContentPtr = Content.GetMutablePtr<FGameItemDropContent>())
	{
		ContentPtr->OnDataChanged();
	}
}
