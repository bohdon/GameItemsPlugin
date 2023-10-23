// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameItemEquipmentComponent.h"

#include "GameItem.h"
#include "GameItemContainerComponent.h"
#include "GameItemDef.h"
#include "Equipment/GameEquipment.h"
#include "Equipment/GameEquipmentDef.h"
#include "Equipment/GameItemFragment_Equipment.h"


UGameItemEquipmentComponent::UGameItemEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TArray<UGameEquipment*> UGameItemEquipmentComponent::FindAllEquipmentFromItem(UGameItem* Item) const
{
	return FindAllEquipmentByInstigator(Item);
}

void UGameItemEquipmentComponent::AddItemContainer(UGameItemContainerComponent* ItemContainer)
{
	if (!ItemContainer || ItemContainers.Contains(ItemContainer))
	{
		return;
	}

	ItemContainers.Add(ItemContainer);

	ItemContainer->OnItemAddedEvent.AddUObject(this, &UGameItemEquipmentComponent::OnItemAdded);
	ItemContainer->OnItemRemovedEvent.AddUObject(this, &UGameItemEquipmentComponent::OnItemRemoved);

	// TODO: apply equipment for items in this new container
}

void UGameItemEquipmentComponent::RemoveItemContainer(UGameItemContainerComponent* ItemContainer)
{
	if (!ItemContainer || !ItemContainers.Contains(ItemContainer))
	{
		return;
	}

	ItemContainers.Remove(ItemContainer);

	ItemContainer->OnItemAddedEvent.RemoveAll(this);
	ItemContainer->OnItemRemovedEvent.RemoveAll(this);
}

void UGameItemEquipmentComponent::ReapplyAllItemEquipment()
{
	RemoveAllEquipment();

	for (TWeakObjectPtr<UGameItemContainerComponent> Container : ItemContainers)
	{
		if (Container.IsValid())
		{
			for (UGameItem* Item : Container->GetAllItems())
			{
				if (ShouldApplyEquipmentForItem(Item))
				{
					ApplyEquipmentForItem(Item);
				}
			}
		}
	}
}

bool UGameItemEquipmentComponent::ShouldApplyEquipmentForItem(UGameItem* Item) const
{
	if (!Item || !Item->GetItemDefCDO())
	{
		return false;
	}

	const UGameItemFragment_Equipment* EquipFrag = Item->GetItemDefCDO()->FindFragment<UGameItemFragment_Equipment>();
	if (!EquipFrag || !EquipFrag->EquipmentDef)
	{
		return false;
	}

	return true;
}

UGameEquipment* UGameItemEquipmentComponent::ApplyEquipmentForItem(UGameItem* Item)
{
	check(Item);
	check(Item->GetItemDefCDO());

	const UGameItemFragment_Equipment* EquipFrag = Item->GetItemDefCDO()->FindFragment<UGameItemFragment_Equipment>();
	if (!EquipFrag)
	{
		return nullptr;
	}

	return ApplyEquipment(EquipFrag->EquipmentDef, Item);
}

void UGameItemEquipmentComponent::RemoveEquipmentForItem(UGameItem* Item)
{
	TArray<UGameEquipment*> ItemEquipment = FindAllEquipmentFromItem(Item);
	for (UGameEquipment* Equipment : ItemEquipment)
	{
		RemoveEquipment(Equipment);
	}
}

void UGameItemEquipmentComponent::OnItemAdded(UGameItem* Item)
{
	if (ShouldApplyEquipmentForItem(Item))
	{
		ApplyEquipmentForItem(Item);
	}
}

void UGameItemEquipmentComponent::OnItemRemoved(UGameItem* Item)
{
	// TODO: consider checking for equip fragment? is that faster than iterating all equipment regardless?
	RemoveEquipmentForItem(Item);
}
