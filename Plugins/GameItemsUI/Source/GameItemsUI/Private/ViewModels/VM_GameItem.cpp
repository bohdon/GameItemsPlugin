// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/VM_GameItem.h"

#include "GameItem.h"
#include "GameItemDef.h"
#include "ViewModels/VM_GameItemTagStat.h"


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
		TagStatViewModels.Reset();

		if (Item)
		{
			Item->OnCountChangedEvent.AddUObject(this, &UVM_GameItem::OnCountChanged);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Item);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetCount);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasMultiple);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetDisplayName);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetAllTagStatViewModels);
	}
}

int32 UVM_GameItem::GetCount() const
{
	return Item ? Item->GetCount() : 0;
}

TArray<UVM_GameItemTagStat*> UVM_GameItem::GetAllTagStatViewModels() const
{
	TArray<UVM_GameItemTagStat*> Result;
	if (!Item)
	{
		return Result;
	}

	for (auto Elem : Item->GetAllTagStats())
	{
		Result.Emplace(GetTagStatViewModel(Elem.Key));
	}
	return Result;
}

UVM_GameItemTagStat* UVM_GameItem::GetTagStatViewModel(FGameplayTag Tag) const
{
	if (!Item || !Item->HasTagStat(Tag))
	{
		return nullptr;
	}

	if (UVM_GameItemTagStat* StatViewModel = TagStatViewModels.FindRef(Tag))
	{
		return StatViewModel;
	}

	// create a new viewmodel
	UVM_GameItem* MutableThis = const_cast<UVM_GameItem*>(this);
	UVM_GameItemTagStat* StatViewModel = NewObject<UVM_GameItemTagStat>(MutableThis);
	StatViewModel->SetItemAndTag(Item, Tag);

	MutableThis->TagStatViewModels.Emplace(Tag, StatViewModel);

	return StatViewModel;
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
