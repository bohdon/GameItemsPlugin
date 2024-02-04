// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DemoInteractionUIInterface.generated.h"

class UDemoInteraction;


UINTERFACE()
class UDemoInteractionUIInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for an object that can show interactions ui.
 */
class GAMEITEMSPLUGIN_API IDemoInteractionUIInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Show(const TArray<UDemoInteraction*>& Interactions);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Hide();
};
