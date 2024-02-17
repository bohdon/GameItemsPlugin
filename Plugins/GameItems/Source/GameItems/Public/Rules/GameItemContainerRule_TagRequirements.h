// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "GameItemContainerRule.h"
#include "GameItemContainerRule_TagRequirements.generated.h"


/**
 * Requires items to match certain tag requirements to be allowed in the container.
 */
UCLASS(DisplayName = "Tag Requirements")
class UGameItemContainerRule_TagRequirements : public UGameItemContainerRule
{
	GENERATED_BODY()

public:
	/** Items must have all of these tags to be permitted in the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Requirements")
	FGameplayTagContainer RequireTags;

	/** Items cannot have any of these tags to be permitted in the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Requirements")
	FGameplayTagContainer IgnoreTags;

	/** Items must match this query to be permitted in the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Requirements")
	FGameplayTagQuery Query;

	virtual bool CanContainItem_Implementation(const UGameItem* Item) const override;
};
