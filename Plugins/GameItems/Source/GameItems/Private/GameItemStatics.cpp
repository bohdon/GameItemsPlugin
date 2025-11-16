// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemStatics.h"

#include "GameItemContainer.h"
#include "GameItemContainerComponent.h"
#include "GameItemDef.h"
#include "GameItemSubsystem.h"
#include "WorldConditionContext.h"
#include "Algo/Accumulate.h"
#include "Conditions/GameItemConditionSchema.h"
#include "DropTable/GameItemDropContent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Equipment/GameItemFragment_Equipment.h"
#include "Fragments/GameItemFragment_DropRules.h"
#include "GameFramework/Actor.h"


UGameItemContainerComponent* UGameItemStatics::GetItemContainerComponentForActor(AActor* Actor)
{
	const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(Actor);
	return ItemSubsystem ? ItemSubsystem->GetContainerComponentForActor(Actor) : nullptr;
}

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

const UGameItemFragment* UGameItemStatics::FindGameItemFragmentFromItem(const UObject* WorldContextObject, UGameItem* Item,
                                                                        TSubclassOf<UGameItemFragment> FragmentClass)
{
	return FindGameItemFragment(WorldContextObject, Item ? Item->GetItemDef() : nullptr, FragmentClass);
}

UGameItemContainer* UGameItemStatics::GetItemContainerById(const TArray<UGameItemContainer*>& Containers, FGameplayTag ContainerId)
{
	for (UGameItemContainer* Container : Containers)
	{
		if (Container->GetContainerId() == ContainerId)
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

	return EvaluateWorldCondition(ItemDefCDO, EquipFrag->Condition, ContextData);
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

	return EvaluateWorldCondition(ItemDefCDO, DropRulesFrag->Condition, ContextData);
}

void UGameItemStatics::SelectItemsFromDropTableRow(const FGameItemDropContext& Context, const FGameItemDropTableRow& DropTableRow,
                                                   TArray<FGameItemDefStack>& OutItems)
{
	if (const FGameItemDropContent* ContentPtr = DropTableRow.Content.GetPtr<FGameItemDropContent>())
	{
		ContentPtr->CheckAndSelectItems(Context, OutItems);
	}
}

int32 UGameItemStatics::GetWeightedRandomArrayIndex(const TArray<float>& Probabilities)
{
	if (Probabilities.IsEmpty())
	{
		return INDEX_NONE;
	}

	const float Total = Algo::Accumulate(Probabilities, 0.f);
	const float Value = FMath::FRand() * Total;

	float Current = 0.f;
	for (int32 Idx = 0; Idx < Probabilities.Num(); ++Idx)
	{
		Current += Probabilities[Idx];
		if (Value <= Current)
		{
			return Idx;
		}
	}

	// return the last index
	return Probabilities.Num() - 1;
}

bool UGameItemStatics::EvaluateWorldCondition(const UObject* Owner, const FWorldConditionQueryDefinition& Condition,
                                              const FWorldConditionContextData& ContextData)
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
