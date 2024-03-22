// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameItemDropParams.generated.h"


/**
 * Parameters for determining how to select items from a drop table.
 * Can be used to affect item quantities and qualities as needed.
 */
USTRUCT(BlueprintType, DisplayName = "Params")
struct FGameItemDropParams
{
	GENERATED_BODY()

	virtual ~FGameItemDropParams() = default;

	/** Open-ended gameplay tag stats which can be used as value multipliers, rarity conditions, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, float> TagStats;
};


/**
 * Item drop params that include a target overall econ value for the content
 * which can be used to calculate item quantities.
 */
USTRUCT(BlueprintType, DisplayName = "Econ Value Params")
struct FGameItemDropParams_EconValue : public FGameItemDropParams
{
	GENERATED_BODY()

	/** The minimum target econ value of the content. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "0"))
	float EconValueMin = 1.f;

	/** The maximum target econ value of the content. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "0"))
	float EconValueMax = 1.f;
};
