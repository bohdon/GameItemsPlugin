// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameItemEquipmentComponent.h"

#include "GameItem.h"
#include "GameItemContainer.h"
#include "GameItemContainerComponent.h"
#include "GameItemsModule.h"
#include "GameItemStatics.h"
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

void UGameItemEquipmentComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (bAutoFindContainerComponent)
	{
		// find item component from owner
		const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
		if (UGameItemContainerComponent* ItemsComponent = ItemSubsystem->GetContainerComponentForActor(GetOwner()))
		{
			RegisterItemContainerComponent(ItemsComponent);
		}
	}
}

void UGameItemEquipmentComponent::UninitializeComponent()
{
	for (auto& Elem : ItemConditionStates)
	{
		const UGameItemConditionSchema* DefaultSchema = GetDefault<UGameItemConditionSchema>();
		FWorldConditionContextData ContextData(*DefaultSchema);

		FGameItemEquipmentConditionState& ItemCondition = Elem.Value;
		const FWorldConditionContext Context(ItemCondition.State, ContextData);
		Context.Deactivate();
	}

	ItemConditionStates.Empty();

	Super::UninitializeComponent();
}

void UGameItemEquipmentComponent::RegisterItemContainerComponent(UGameItemContainerComponent* ItemContainerComponent)
{
	if (!ItemContainerComponent || RegisteredContainerComponents.Contains(ItemContainerComponent))
	{
		return;
	}

	RegisteredContainerComponents.Add(ItemContainerComponent);

	UE_LOG(LogGameItems, Verbose, TEXT("%s Registered item component: %s"),
		*GetDebugPrefix(), *ItemContainerComponent->GetReadableName());

	ItemContainerComponent->OnItemAddedEvent.AddUObject(this, &ThisClass::OnItemAdded);
	ItemContainerComponent->OnItemRemovedEvent.AddUObject(this, &ThisClass::OnItemRemoved);

	// activate any existing items
	for (const UGameItemContainer* Container : ItemContainerComponent->GetAllItemContainers())
	{
		if (Container)
		{
			for (const auto& Elem : Container->GetAllItems())
			{
				OnItemAdded(Elem.Value);
			}
		}
	}
}

void UGameItemEquipmentComponent::UnregisterItemContainerComponent(UGameItemContainerComponent* ItemContainerComponent)
{
	if (!ItemContainerComponent || !RegisteredContainerComponents.Contains(ItemContainerComponent))
	{
		return;
	}

	RegisteredContainerComponents.Remove(ItemContainerComponent);

	UE_LOG(LogGameItems, Verbose, TEXT("%s Unregistered item component: %s"),
		*GetDebugPrefix(), *ItemContainerComponent->GetReadableName());

	ItemContainerComponent->OnItemAddedEvent.RemoveAll(this);
	ItemContainerComponent->OnItemRemovedEvent.RemoveAll(this);

	// remove all items
	for (const UGameItemContainer* Container : ItemContainerComponent->GetAllItemContainers())
	{
		if (Container)
		{
			for (const auto& Elem : Container->GetAllItems())
			{
				OnItemRemoved(Elem.Value);
			}
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

	Item->OnSlottedEvent.Add(UGameItem::FSlottedDelegate::FDelegate::CreateUObject(this, &ThisClass::OnExistingItemSlotted, Item));
	Item->OnUnslottedEvent.Add(UGameItem::FUnslottedDelegate::FDelegate::CreateUObject(this, &ThisClass::OnExistingItemUnslotted, Item));

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
		UE_LOG(LogGameItems, Error, TEXT("[%s] Failed to activate condition for item equipment: %s"),
			*GetReadableName(), *Item->GetName());
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

	FGameItemEquipmentConditionState& ItemCondition = ItemConditionStates.FindChecked(Item);

	// setup condition context
	const UGameItemConditionSchema* DefaultSchema = GetDefault<UGameItemConditionSchema>();
	FWorldConditionContextData ContextData(*DefaultSchema);
	SetupConditionContextData(ContextData, Item);

	// check the condition immediately
	const FWorldConditionContext Context(ItemCondition.State, ContextData);
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

void UGameItemEquipmentComponent::ApplyEquipmentForItem(UGameItem* Item)
{
	const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item);
	if (!EquipFrag->EquipmentDef)
	{
		return;
	}

	ItemEquipmentDefs.Add(Item, EquipFrag->EquipmentDef);

	// copy all tag stats from the item, this allows applying equipment on the server
	// without having to replicate local-only items.
	const FGameEquipmentSpec Spec = FGameEquipmentSpec(EquipFrag->EquipmentDef, Item->GetTagStatsContainer());
	ApplyEquipmentSpec(Spec);
}

void UGameItemEquipmentComponent::RemoveEquipmentForItem(UGameItem* Item)
{
	// items must have unique equipment defs (there's no other association between item and equipment),
	// this limitation also exists allow local-only items but server-spawned equipment.

	if (const TSubclassOf<UGameEquipmentDef>* EquipmentDef = ItemEquipmentDefs.Find(Item))
	{
		RemoveEquipmentByDef(*EquipmentDef);
		ItemEquipmentDefs.Remove(Item);
	}
}

UGameEquipment* UGameItemEquipmentComponent::FindAllEquipmentFromItem(UGameItem* Item) const
{
	return FindEquipmentByDef(ItemEquipmentDefs.FindRef(Item));
}

const UGameItemFragment_Equipment* UGameItemEquipmentComponent::GetItemEquipmentFragment_Implementation(UGameItem* Item) const
{
	if (const UGameItemFragment_Equipment* EquipFrag = UGameItemStatics::FindFragmentFromItem<UGameItemFragment_Equipment>(Item))
	{
		if (EquipFrag->EquipmentDef)
		{
			return EquipFrag;
		}
	}
	return nullptr;
}

bool UGameItemEquipmentComponent::ShouldHandleItemEquipment_Implementation(UGameItem* Item) const
{
	return true;
}

void UGameItemEquipmentComponent::OnItemAdded(UGameItem* Item)
{
	if (!ShouldHandleItemEquipment(Item))
	{
		return;
	}

	if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
	{
		ActivateItemEquipmentCondition(Item, EquipFrag);
	}
}

void UGameItemEquipmentComponent::OnItemRemoved(UGameItem* Item)
{
	if (!ShouldHandleItemEquipment(Item))
	{
		return;
	}

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
