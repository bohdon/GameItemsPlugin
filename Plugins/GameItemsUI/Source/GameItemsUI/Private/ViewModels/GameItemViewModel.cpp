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
		if (Item)
		{
			Item->OnCountChangedEvent.RemoveAll(this);
		}

		Item = NewItem;

		if (Item)
		{
			Item->OnCountChangedEvent.AddUObject(this, &UGameItemViewModel::OnCountChanged);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Item);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCount);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasMultiple);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDisplayName);
	}
}

int32 UGameItemViewModel::GetCount() const
{
	return Item ? Item->GetCount() : 0;
}

bool UGameItemViewModel::HasMultiple() const
{
	return GetCount() > 1;
}

FText UGameItemViewModel::GetDisplayName() const
{
	return Item ? Item->GetItemDefCDO()->DisplayName : FText::GetEmpty();
}

void UGameItemViewModel::OnCountChanged(int32 NewCount, int32 OldCount)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCount);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasMultiple);
}
