// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerProvider.h"
#include "GameItemContainerProvider_DemoInteraction.generated.h"


/**
 * Provides an item container from the current interaction context.
 */
UCLASS(DisplayName = "Demo Interaction Context")
class GAMEITEMSPLUGIN_API UGameItemContainerProvider_DemoInteraction : public UGameItemContainerProvider
{
	GENERATED_BODY()

public:
	virtual UGameItemContainer* ProvideContainer_Implementation(const FGameplayTag& ContainerId,
	                                                            const FGameItemViewContext& ContextData) const override;
};
