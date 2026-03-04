// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "GameItemFragment_Usage.generated.h"

class AActor;


/**
 * Base class for fragments that can handle an item being "used". 
 */
UCLASS(Abstract)
class GAMEITEMS_API UGameItemFragment_Usage : public UGameItemFragment
{
	GENERATED_BODY()

public:
	/** Use an item with this fragment. */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	bool Use(UGameItem* Item, AActor* Instigator) const;
};
