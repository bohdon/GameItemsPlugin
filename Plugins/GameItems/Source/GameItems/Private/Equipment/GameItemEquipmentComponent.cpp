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
#if WITH_SERVER_CODE
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!ItemContainer || ItemContainers.Contains(ItemContainer))
	{
		return;
	}

	ItemContainers.Add(ItemContainer);

	ItemContainer->OnItemAddedEvent.AddUObject(this, &UGameItemEquipmentComponent::OnItemAdded);
	ItemContainer->OnItemRemovedEvent.AddUObject(this, &UGameItemEquipmentComponent::OnItemRemoved);

	for (const TPair<int32, UGameItem*>& Elem : ItemContainer->GetAllItems())
	{
		OnItemAdded(Elem.Value);
	}
#endif
}

void UGameItemEquipmentComponent::RemoveItemContainer(UGameItemContainer* ItemContainer)
{
#if WITH_SERVER_CODE
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!ItemContainer || !ItemContainers.Contains(ItemContainer))
	{
		return;
	}

	ItemContainers.Remove(ItemContainer);

	ItemContainer->OnItemAddedEvent.RemoveAll(this);
	ItemContainer->OnItemRemovedEvent.RemoveAll(this);

	for (const TPair<int32, UGameItem*>& Elem : ItemContainer->GetAllItems())
	{
		OnItemRemoved(Elem.Value);
	}
#endif
}

void UGameItemEquipmentComponent::FindAllItemContainers(FGameplayTagQuery Query, bool bIgnoreChildContainers)
{
	const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
	const TArray<UGameItemContainer*> AllContainers = ItemSubsystem->GetAllContainersForActor(GetOwner());
	for (UGameItemContainer* Container : AllContainers)
	{
		if (bIgnoreChildContainers && Container->IsChild())
		{
			continue;
		}
		if (Query.IsEmpty() || Query.Matches(FGameplayTagContainer(Container->GetContainerId())))
		{
			AddItemContainer(Container);
		}
	}

	// TODO: add support for auto-adding new containers that match the query
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

#if WITH_SERVER_CODE
	if (GetOwner()->HasAuthority())
	{
		if (bAutoFindContainers)
		{
			FindAllItemContainers(DefaultContainerQuery, true);
		}
	}
#endif
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

void UGameItemEquipmentComponent::ActivateItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag)
{
#if WITH_SERVER_CODE
	check(Item);
	check(EquipFrag);

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

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
		UE_LOG(LogGameItems, Error, TEXT("[%s] Failed to activate condition for item equipment: %s"),
		       *GetReadableName(), *Item->GetName());
		return;
	}

	// check the condition immediately
	if (Context.IsTrue())
	{
		ApplyEquipmentForItem(Item);
	}
#endif
}

void UGameItemEquipmentComponent::DeactivateItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag)
{
#if WITH_SERVER_CODE
	check(Item);
	check(EquipFrag);

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

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
#endif
}

void UGameItemEquipmentComponent::CheckItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag)
{
#if WITH_SERVER_CODE
	check(Item);
	check(EquipFrag);

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

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
#endif
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
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	if (ItemEquipmentMap.Contains(Item))
	{
		// equipment already applied
		return;
	}

	const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item);
	UGameEquipment* NewEquipment = ApplyEquipment(EquipFrag->EquipmentDef, Item);
	if (NewEquipment)
	{
		ItemEquipmentMap.Add(Item, NewEquipment);
	}
#endif
}

void UGameItemEquipmentComponent::RemoveEquipmentForItem(UGameItem* Item)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	TArray<UGameEquipment*> ItemEquipment = FindAllEquipmentFromItem(Item);
	for (UGameEquipment* Equipment : ItemEquipment)
	{
		RemoveEquipment(Equipment);
	}

	ItemEquipmentMap.Remove(Item);
#endif
}

void UGameItemEquipmentComponent::OnItemAdded(UGameItem* Item)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
	{
		ActivateItemEquipmentCondition(Item, EquipFrag);
	}
#endif
}

void UGameItemEquipmentComponent::OnItemRemoved(UGameItem* Item)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
	{
		DeactivateItemEquipmentCondition(Item, EquipFrag);
	}
#endif
}

void UGameItemEquipmentComponent::OnExistingItemSlotted(const UGameItemContainer* Container, int32 NewSlot, int32 OldSlot, UGameItem* Item)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	if (Item && ItemConditionStates.Find(Item))
	{
		if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
		{
			CheckItemEquipmentCondition(Item, EquipFrag);
		}
	}
#endif
}

void UGameItemEquipmentComponent::OnExistingItemUnslotted(const UGameItemContainer* Container, int32 OldSlot, UGameItem* Item)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	if (Item && ItemConditionStates.Find(Item))
	{
		if (const UGameItemFragment_Equipment* EquipFrag = GetItemEquipmentFragment(Item))
		{
			CheckItemEquipmentCondition(Item, EquipFrag);
		}
	}
#endif
}
