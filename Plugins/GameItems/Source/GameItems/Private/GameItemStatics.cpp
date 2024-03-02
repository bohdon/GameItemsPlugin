// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemStatics.h"

#include "GameItemContainer.h"
#include "GameItemDef.h"
#include "GameItemSubsystem.h"
#include "WorldConditionContext.h"
#include "Conditions/GameItemConditionSchema.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Equipment/GameItemFragment_Equipment.h"
#include "Fragments/GameItemFragment_DropRules.h"


TArray<UGameItemContainer*> UGameItemStatics::GetAllItemContainersForActor(AActor* Actor)
{
	const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(Actor);
	return ItemSubsystem ? ItemSubsystem->GetAllContainersForActor(Actor) : TArray<UGameItemContainer*>();
}

UGameItemContainer* UGameItemStatics::GetItemContainerForActor(AActor* Actor, FGameplayTag ContainerId)
{
	const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(Actor);
	return ItemSubsystem ? ItemSubsystem->GetContainerForActor(Actor, ContainerId) : nullptr;
}

const UGameItemFragment* UGameItemStatics::FindGameItemFragment(const UObject* WorldContextObject, TSubclassOf<UGameItemDef> ItemDef,
                                                                TSubclassOf<UGameItemFragment> FragmentClass)
{
	const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(WorldContextObject);
	return ItemSubsystem ? ItemSubsystem->FindFragment(ItemDef, FragmentClass) : nullptr;
}

UGameItemContainer* UGameItemStatics::GetItemContainerById(const TArray<UGameItemContainer*>& Containers, FGameplayTag ContainerId)
{
	for (UGameItemContainer* Container : Containers)
	{
		if (Container->ContainerId == ContainerId)
		{
			return Container;
		}
	}
	return nullptr;
}

bool UGameItemStatics::IsEquipmentConditionMet(UGameItem* Item)
{
	if (!Item || !Item->GetItemDefCDO())
	{
		return false;
	}

	const UGameItemDef* ItemDefCDO = Item->GetItemDefCDO();
	const UGameItemFragment_Equipment* EquipFrag = ItemDefCDO->FindFragment<UGameItemFragment_Equipment>();

	// setup condition context
	const UGameItemConditionSchema* DefaultSchema = GetDefault<UGameItemConditionSchema>();
	FWorldConditionContextData ContextData(*DefaultSchema);
	ContextData.SetContextData(DefaultSchema->GetTargetItemRef(), Item);

	return EvaluateCondition(ItemDefCDO, EquipFrag->Condition, ContextData);
}

bool UGameItemStatics::IsDropConditionMet(TSubclassOf<UGameItemDef> ItemDef, AActor* TargetActor)
{
	if (!ItemDef)
	{
		return false;
	}

	const UGameItemDef* ItemDefCDO = GetDefault<UGameItemDef>(ItemDef);
	const UGameItemFragment_DropRules* DropRulesFrag = ItemDefCDO->FindFragment<UGameItemFragment_DropRules>();

	// setup condition context
	const UGameItemConditionSchema* DefaultSchema = GetDefault<UGameItemConditionSchema>();
	FWorldConditionContextData ContextData(*DefaultSchema);
	ContextData.SetContextData(DefaultSchema->GetTargetActorRef(), TargetActor);

	return EvaluateCondition(ItemDefCDO, DropRulesFrag->Condition, ContextData);
}

bool UGameItemStatics::EvaluateCondition(const UObject* Owner, const FWorldConditionQueryDefinition& Condition, const FWorldConditionContextData& ContextData)
{
	FWorldConditionQueryState QueryState;
	QueryState.Initialize(*Owner, Condition);

	const FWorldConditionContext Context(QueryState, ContextData);
	if (!Context.Activate())
	{
		return false;
	}

	const bool bIsTrue = Context.IsTrue();

	Context.Deactivate();

	return bIsTrue;
}
