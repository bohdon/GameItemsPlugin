// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "GameplayTagContainer.h"
#include "GameItemFragment_TagStats.generated.h"


/**
 * Defines starting tag-based stats, like level or ammo.
 */
UCLASS(DisplayName = "Tag Stats")
class GAMEITEMS_API UGameItemFragment_TagStats : public UGameItemFragment
{
	GENERATED_BODY()

public:
	/** The default stats for this item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TagStats", meta = (GameplayTagFilter = "GameItemStatTagsCategory"))
	TMap<FGameplayTag, int32> DefaultStats;

	virtual void OnItemCreated(UGameItem* Item) const override;
};
