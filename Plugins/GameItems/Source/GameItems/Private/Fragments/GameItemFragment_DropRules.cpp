// Copyright Bohdon Sayre, All Rights Reserved.


#include "Fragments/GameItemFragment_DropRules.h"

#include "GameItemDef.h"
#include "GameItemStatics.h"
#include "WorldConditionContext.h"
#include "Conditions/GameItemConditionSchema.h"


UGameItemFragment_DropRules::UGameItemFragment_DropRules(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  Probability(1.f)
{
	Condition.SetSchemaClass(UGameItemConditionSchema::StaticClass());
}

float UGameItemFragment_DropRules::GetProbability(const FGameItemDropContext& Context) const
{
	return Probability;
}

bool UGameItemFragment_DropRules::IsConditionMet(const FGameItemDropContext& Context) const
{
	// setup condition context
	const UGameItemConditionSchema* DefaultSchema = GetDefault<UGameItemConditionSchema>();
	FWorldConditionContextData ContextData(*DefaultSchema);
	ContextData.SetContextData<UObject>(DefaultSchema->GetTargetActorRef(), Context.TargetActor);

	return UGameItemStatics::EvaluateWorldCondition(this, Condition, ContextData);
}
