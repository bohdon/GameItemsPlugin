// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "Engine/DataAsset.h"
#include "GameItemDef.generated.h"


/**
 * Base class for a gameplay item definition. Designed to be subclassed in Blueprint,
 * then filled out using GameItemFragments.
 */
UCLASS(Blueprintable, Abstract)
class GAMEITEMS_API UGameItemDef : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameItem")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "GameItem")
	TArray<TObjectPtr<UGameItemFragment>> Fragments;
};
