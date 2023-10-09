// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "Engine/DataAsset.h"
#include "GameItemSet.generated.h"


/**
 * A collection of game items, as definitions and quantities,
 * e.g. for use in predefining containers or groups of items to give out.
 */
UCLASS(BlueprintType, Blueprintable)
class GAMEITEMS_API UGameItemSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The items in the set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (TitleProperty = "{Count} {ItemDef}"))
	TArray<FGameItemDefStack> Items;

	/** Add all items in this set to a container. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	virtual void AddItemsToContainer(UGameItemContainerComponent* ItemContainer) const;
};
