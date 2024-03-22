// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "WorldConditionQuery.h"
#include "DropTable/GameItemDropContext.h"
#include "GameItemFragment_DropRules.generated.h"

class UGameItemDef;


/**
 * Defines stats and conditions about the item that can affect how it is selected from drop tables.
 */
UCLASS(DisplayName = "Drop Rules")
class GAMEITEMS_API UGameItemFragment_DropRules : public UGameItemFragment
{
	GENERATED_BODY()

public:
	UGameItemFragment_DropRules(const FObjectInitializer& ObjectInitializer);

	/** The relative probability of selecting this item in a drop table. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "0"), Category = "DropRules")
	float Probability;

	/** Conditions that must be met for the item to be selected. */
	UPROPERTY(EditAnywhere, Category = "DropRules")
	FWorldConditionQueryDefinition Condition;

	virtual float GetProbability(const FGameItemDropContext& Context) const;

	virtual bool IsConditionMet(const FGameItemDropContext& Context) const;
};
