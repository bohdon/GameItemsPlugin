// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "GameItemFragment_EconValue.generated.h"


/**
 * Defines a globally comparable value of the item for the purposes of
 * converting item quantities and currencies.
 */
UCLASS(DisplayName = "Econ Value")
class GAMEITEMS_API UGameItemFragment_EconValue : public UGameItemFragment
{
	GENERATED_BODY()

public:
	/** The globally comparable value of the item for the purposes of converting item quantities and currencies. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "0"))
	float EconValue = 1.f;

	/** Return the quantity of an item to use given a target econ value. */
	virtual int32 GetCountForValue(float TargetValue, int32 MinCount = 1) const;
};
