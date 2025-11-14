// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "Slate/SlateBrushAsset.h"
#include "GameItemFragment_UIData.generated.h"

class UGameItemDef;


/**
 * Defines common UI data for an item, like description and icon.
 */
UCLASS(DisplayName = "UI Data")
class GAMEITEMS_API UGameItemFragment_UIData : public UGameItemFragment
{
	GENERATED_BODY()

public:
	/** The item description. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIData")
	FText Description;

	/** The icon of this item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIData")
	TObjectPtr<USlateBrushAsset> Icon;
};
