// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/VM_GameItemTagStat.h"

#include "GameItem.h"


void UVM_GameItemTagStat::SetItemAndTag(UGameItem* NewItem, FGameplayTag NewTag)
{
	if (NewItem != Item)
	{
		if (Item)
		{
			Item->OnTagStatChangedEvent.RemoveAll(this);
		}

		Item = NewItem;

		if (Item)
		{
			Item->OnTagStatChangedEvent.AddUObject(this, &ThisClass::OnTagStatChanged);
		}
	}

	Tag = NewTag;

	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Item);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Tag);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetValue);
}

void UVM_GameItemTagStat::SetTag(const FGameplayTag& NewTag)
{
	UE_MVVM_SET_PROPERTY_VALUE(Tag, NewTag);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetValue);
}

int32 UVM_GameItemTagStat::GetValue() const
{
	return Item ? Item->GetTagStat(Tag) : 0;
}

void UVM_GameItemTagStat::OnTagStatChanged(const FGameplayTag& ChangedTag, int32 NewValue, int32 OldValue)
{
	if (ChangedTag == Tag)
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetValue);
	}
}
