// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/VM_GameItem.h"

#include "GameItem.h"
#include "GameItemDef.h"


UVM_GameItem::UVM_GameItem()
{
}

void UVM_GameItem::SetItem(UGameItem* NewItem)
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
			Item->OnCountChangedEvent.AddUObject(this, &UVM_GameItem::OnCountChanged);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Item);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCount);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasMultiple);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDisplayName);
	}
}

int32 UVM_GameItem::GetCount() const
{
	return Item ? Item->GetCount() : 0;
}

bool UVM_GameItem::HasMultiple() const
{
	return GetCount() > 1;
}

FText UVM_GameItem::GetDisplayName() const
{
	return Item ? Item->GetItemDefCDO()->DisplayName : FText::GetEmpty();
}

void UVM_GameItem::OnCountChanged(int32 NewCount, int32 OldCount)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCount);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasMultiple);
}
