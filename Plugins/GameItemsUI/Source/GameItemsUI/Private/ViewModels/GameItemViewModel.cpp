// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/GameItemViewModel.h"

#include "GameItem.h"
#include "GameItemDef.h"


UGameItemViewModel::UGameItemViewModel()
{
}

void UGameItemViewModel::SetItem(UGameItem* NewItem)
{
	if (NewItem)
	{
		// make sure only valid items are set
		if (!ensureAlwaysMsgf(NewItem->GetItemDef(), TEXT("Cant use invalid item: %s"), *NewItem->GetName()))
		{
			return;
		}
	}

	if (Item != NewItem)
	{
		UE_MVVM_SET_PROPERTY_VALUE(Item, NewItem);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDisplayName);
	}
}

FText UGameItemViewModel::GetDisplayName() const
{
	return Item ? Item->GetItemDefCDO()->DisplayName : FText::GetEmpty();
}
