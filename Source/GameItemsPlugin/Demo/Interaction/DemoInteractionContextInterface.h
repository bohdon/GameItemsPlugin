// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DemoInteraction.h"
#include "UObject/Interface.h"
#include "DemoInteractionContextInterface.generated.h"


UINTERFACE()
class UDemoInteractionContextInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for an object that should behave specific to an interaction.
 * Allows passing through the interactor and interaction components as context.
 */
class GAMEITEMSPLUGIN_API IDemoInteractionContextInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetInteractionContext(const FDemoInteractionContext& Context);
};
