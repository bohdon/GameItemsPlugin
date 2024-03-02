// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "WorldConditionQuery.h"
#include "GameItemFragment_DropRules.generated.h"


/**
 * Defines stats and conditions about the item that can affect how it is selected from drop tables.
 */
UCLASS()
class GAMEITEMS_API UGameItemFragment_DropRules : public UGameItemFragment
{
	GENERATED_BODY()

public:
	UGameItemFragment_DropRules(const FObjectInitializer& ObjectInitializer);

	/** How likely this item is to be selected in a drop table. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "0"), Category = "DropRules")
	float Rate;

	/** Conditions that must be met for the item to be selected. */
	UPROPERTY(EditAnywhere, Category = "DropRules")
	FWorldConditionQueryDefinition Condition;
};
