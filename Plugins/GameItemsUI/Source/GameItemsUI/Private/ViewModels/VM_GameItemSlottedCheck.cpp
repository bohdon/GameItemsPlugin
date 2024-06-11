// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/VM_GameItemSlottedCheck.h"

#include "GameItem.h"
#include "GameItemDef.h"


void UVM_GameItemSlottedCheck::SetContainerTags(FGameplayTagContainer NewContainerTags)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(ContainerTags, NewContainerTags))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotted);
	}
}

void UVM_GameItemSlottedCheck::SetItem(UGameItem* NewItem)
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
			Item->OnSlottedEvent.AddUObject(this, &UVM_GameItemSlottedCheck::OnSlotted);
			Item->OnUnslottedEvent.AddUObject(this, &UVM_GameItemSlottedCheck::OnUnslotted);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Item);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotted);
	}
}

bool UVM_GameItemSlottedCheck::IsSlotted() const
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

void UVM_GameItemSlottedCheck::OnSlotted(const UGameItemContainer* Container, int32 NewSlot, int32 OldSlot)
{
	if (Container->GetOwnedTags().HasAny(ContainerTags))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotted);
	}
}

void UVM_GameItemSlottedCheck::OnUnslotted(const UGameItemContainer* Container, int32 OldSlot)
{
	if (Container->GetOwnedTags().HasAny(ContainerTags))
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsSlotted);
	}
}
