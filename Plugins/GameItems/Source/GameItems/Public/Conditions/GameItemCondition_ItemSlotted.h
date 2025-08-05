// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemConditionBase.h"
#include "GameplayTagContainer.h"
#include "GameItemCondition_ItemSlotted.generated.h"


/**
 * A condition that is true when the target item is slotted in a matching container.
 */
USTRUCT(meta=(DisplayName="Item Slotted"))
struct FGameItemCondition_ItemSlotted : public FGameItemConditionBase
{
	GENERATED_BODY()

	/** The condition will be true if the item is slotted in a container with any of these tags. */
	UPROPERTY(EditAnywhere, Category = "Default", meta = (GameplayTagFilter = "GameItemContainerTagsCategory"))
	FGameplayTagContainer ContainerTags;

protected:
#if WITH_EDITOR
	virtual FText GetDescription() const override;
#endif

	virtual bool Initialize(const UWorldConditionSchema& Schema) override;
	virtual FWorldConditionResult IsTrue(const FWorldConditionContext& Context) const override;
};
