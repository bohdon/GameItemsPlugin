// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameItemEquipmentComponent.h"

#include "GameItem.h"
#include "GameItemContainer.h"
#include "GameItemDef.h"
#include "GameItemsModule.h"
#include "GameItemSubsystem.h"
#include "WorldConditionContext.h"
#include "Conditions/GameItemConditionSchema.h"
#include "Equipment/GameEquipment.h"
#include "Equipment/GameEquipmentDef.h"
#include "Equipment/GameItemFragment_Equipment.h"
#include "GameFramework/Actor.h"


UGameItemEquipmentComponent::UGameItemEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TArray<UGameEquipment*> UGameItemEquipmentComponent::FindAllEquipmentFromItem(UGameItem* Item) const
{
	return FindAllEquipmentByInstigator(Item);
}

void UGameItemEquipmentComponent::AddItemContainer(UGameItemContainer* ItemContainer)
{
	if (!ItemContainer || ItemContainers.Contains(ItemContainer))
	{
		return;
	}

	ItemContainers.Add(ItemContainer);

	ItemContainer->OnItemAddedEvent.AddUObject(this, &UGameItemEquipmentComponent::OnItemAdded);
	ItemContainer->OnItemRemovedEvent.AddUObject(this, &UGameItemEquipmentComponent::OnItemRemoved);

	for (UGameItem* Item : ItemContainer->GetAllItems())
	{
		OnItemAdded(Item);
	}
}

void UGameItemEquipmentComponent::RemoveItemContainer(UGameItemContainer* ItemContainer)
{
	if (!ItemContainer || !ItemContainers.Contains(ItemContainer))
	{
		return;
	}

	ItemContainers.Remove(ItemContainer);

	ItemContainer->OnItemAddedEvent.RemoveAll(this);
	ItemContainer->OnItemRemovedEvent.RemoveAll(this);

	for (UGameItem* Item : ItemContainer->GetAllItems())
	{
		OnItemRemoved(Item);
	}
}

void UGameItemEquipmentComponent::ReapplyAllItemEquipment()
{
	RemoveAllEquipment();

	for (TWeakObjectPtr<UGameItemContainer> Container : ItemContainers)
	{
		if (Container.IsValid())
		{
			for (UGameItem* Item : Container->GetAllItems())
			{
				if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
				{
					CheckItemEquipmentCondition(Item, EquipFrag);
				}
			}
		}
	}
}

const UGameItemFragment_Equipment* UGameItemEquipmentComponent::GetItemEquipmentFragment(UGameItem* Item) const
{
	if (Item)
	{
		if (const UGameItemDef* ItemDefCDO = Item->GetItemDefCDO())
		{
			const UGameItemFragment_Equipment* EquipFrag = ItemDefCDO->FindFragment<UGameItemFragment_Equipment>();
			if (EquipFrag && EquipFrag->EquipmentDef)
			{
				return EquipFrag;
			}
		}
	}

	return nullptr;
}

void UGameItemEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
	for (const FGameplayTag ContainerId : StartupContainerIds)
	{
		if (UGameItemContainer* Container = ItemSubsystem->GetContainerForActor(GetOwner(), ContainerId))
		{
			AddItemContainer(Container);
		}
	}
}

void UGameItemEquipmentComponent::ActivateItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag)
{
	check(Item);
	check(EquipFrag);

	if (!EquipFrag->Condition.IsValid())
	{
		// no condition, apply equipment immediately
		ApplyEquipmentForItem(Item);
		return;
	}

	Item->OnSlottedEvent.Add(UGameItem::FSlottedDelegate::FDelegate::CreateUObject(this, &UGameItemEquipmentComponent::OnExistingItemSlotted, Item));
	Item->OnUnslottedEvent.Add(UGameItem::FUnslottedDelegate::FDelegate::CreateUObject(this, &UGameItemEquipmentComponent::OnExistingItemUnslotted, Item));

	FGameItemEquipmentConditionState& ItemCondition = ItemConditionStates.FindOrAdd(Item);

	// setup condition context
	const UGameItemConditionSchema* DefaultSchema = GetDefault<UGameItemConditionSchema>();
	FWorldConditionContextData ContextData(*DefaultSchema);
	SetupConditionContextData(ContextData, Item);

	ItemCondition.State.Initialize(*this, EquipFrag->Condition);

	// activate
	const FWorldConditionContext Context(ItemCondition.State, ContextData);
	if (!Context.Activate())
	{
		UE_LOG(LogGameItems, Error, TEXT("Failed to activate condition for item equipment: %s"), *Item->GetName());
		return;
	}

	// check the condition immediately
	if (Context.IsTrue())
	{
		ApplyEquipmentForItem(Item);
	}
}

void UGameItemEquipmentComponent::DeactivateItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag)
{
	check(Item);
	check(EquipFrag);

	// always remove equipment
	RemoveEquipmentForItem(Item);

	if (!EquipFrag->Condition.IsValid())
	{
		// no condition
		return;
	}

	Item->OnSlottedEvent.RemoveAll(this);
	Item->OnUnslottedEvent.RemoveAll(this);

	FGameItemEquipmentConditionState& ItemCondition = ItemConditionStates.FindChecked(Item);

	// setup condition context
	const UGameItemConditionSchema* DefaultSchema = GetDefault<UGameItemConditionSchema>();
	FWorldConditionContextData ContextData(*DefaultSchema);
	SetupConditionContextData(ContextData, Item);

	// deactivate and remove
	const FWorldConditionContext Context(ItemCondition.State, ContextData);
	Context.Deactivate();

	ItemConditionStates.Remove(Item);
}

void UGameItemEquipmentComponent::CheckItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag)
{
	check(Item);
	check(EquipFrag);

	FGameItemEquipmentConditionState* ItemCondition = ItemConditionStates.Find(Item);

	if (!ItemCondition)
	{
		// no item condition
		ApplyEquipmentForItem(Item);
	}

	// setup condition context
	const UGameItemConditionSchema* DefaultSchema = GetDefault<UGameItemConditionSchema>();
	FWorldConditionContextData ContextData(*DefaultSchema);
	SetupConditionContextData(ContextData, Item);

	// check the condition immediately
	const FWorldConditionContext Context(ItemCondition->State, ContextData);
	if (Context.IsTrue())
	{
		ApplyEquipmentForItem(Item);
	}
	else
	{
		RemoveEquipmentForItem(Item);
	}
}

void UGameItemEquipmentComponent::SetupConditionContextData(FWorldConditionContextData& ContextData, const UGameItem* Item) const
{
	const UGameItemConditionSchema* DefaultSchema = GetDefault<UGameItemConditionSchema>();
	ContextData.SetContextData(DefaultSchema->GetSubsystemRef(), UGameItemSubsystem::GetGameItemSubsystem(this));
	ContextData.SetContextData(DefaultSchema->GetTargetItemRef(), Item);
	ContextData.SetContextData(DefaultSchema->GetTargetActorRef(), GetOwner());
}

UGameEquipment* UGameItemEquipmentComponent::ApplyEquipmentForItem(UGameItem* Item)
{
	if (UGameEquipment* ExistingEquipment = ItemEquipmentMap.FindRef(Item))
	{
		// equipment already applied
		return ExistingEquipment;
	}

	const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item);
	auto NewEquipment = ApplyEquipment(EquipFrag->EquipmentDef, Item);

	ItemEquipmentMap.Add(Item, NewEquipment);
	return NewEquipment;
}

void UGameItemEquipmentComponent::RemoveEquipmentForItem(UGameItem* Item)
{
	TArray<UGameEquipment*> ItemEquipment = FindAllEquipmentFromItem(Item);
	for (UGameEquipment* Equipment : ItemEquipment)
	{
		RemoveEquipment(Equipment);
	}

	ItemEquipmentMap.Remove(Item);
}

void UGameItemEquipmentComponent::OnItemAdded(UGameItem* Item)
{
	if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
	{
		ActivateItemEquipmentCondition(Item, EquipFrag);
	}
}

void UGameItemEquipmentComponent::OnItemRemoved(UGameItem* Item)
{
	if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
	{
		DeactivateItemEquipmentCondition(Item, EquipFrag);
	}
}

void UGameItemEquipmentComponent::OnExistingItemSlotted(const UGameItemContainer* Container, int32 NewSlot, int32 OldSlot, UGameItem* Item)
{
	if (Item && ItemConditionStates.Find(Item))
	{
		if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
		{
			CheckItemEquipmentCondition(Item, EquipFrag);
		}
	}
}

void UGameItemEquipmentComponent::OnExistingItemUnslotted(const UGameItemContainer* Container, int32 OldSlot, UGameItem* Item)
{
	if (Item && ItemConditionStates.Find(Item))
	{
		if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
		{
			CheckItemEquipmentCondition(Item, EquipFrag);
		}
	}
}
