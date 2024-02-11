// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/GameItemSlottedCheckViewModel.h"

#include "GameItem.h"
#include "GameItemDef.h"


void UGameItemSlottedCheckViewModel::SetContainerTags(FGameplayTagContainer NewContainerTags)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ContainerTags, NewContainerTags))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotted);
	}
}

void UGameItemSlottedCheckViewModel::SetItem(UGameItem* NewItem)
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
			Item->OnSlottedEvent.RemoveAll(this);
			Item->OnUnslottedEvent.RemoveAll(this);
		}

		Item = NewItem;

		if (Item)
		{
			Item->OnSlottedEvent.AddUObject(this, &UGameItemSlottedCheckViewModel::OnSlotted);
			Item->OnUnslottedEvent.AddUObject(this, &UGameItemSlottedCheckViewModel::OnUnslotted);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Item);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotted);
	}
}

bool UGameItemSlottedCheckViewModel::IsSlotted() const
{
	if (!Item)
	{
		return false;
	}

	TArray<UGameItemContainer*> Containers = Item->GetContainers();
	for (UGameItemContainer* Container : Containers)
	{
		if (Container->GetOwnedTags().HasAny(ContainerTags))
		{
			return true;
		}
	}
	return false;
}

void UGameItemSlottedCheckViewModel::OnSlotted(const UGameItemContainer* Container, int32 NewSlot, int32 OldSlot)
{
	if (Container->GetOwnedTags().HasAny(ContainerTags))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotted);
	}
}

void UGameItemSlottedCheckViewModel::OnUnslotted(const UGameItemContainer* Container, int32 OldSlot)
{
	if (Container->GetOwnedTags().HasAny(ContainerTags))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotted);
	}
}
