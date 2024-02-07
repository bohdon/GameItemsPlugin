// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DemoInteractionTypes.h"
#include "UObject/Object.h"
#include "DemoInteraction.generated.h"

class UDemoInteractionComponent;
class UInputAction;


/**
 * An action that can be performed when interacting with an object.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class GAMEITEMSPLUGIN_API UDemoInteraction : public UObject
{
	GENERATED_BODY()

public:
	UDemoInteraction(const FObjectInitializer& ObjectInitializer);

	/** The input action used to trigger this interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UInputAction> InputAction;

	/** The description of the interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Trigger the interaction and return true if successful. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool Trigger(const FDemoInteractionContext& Context);

public:
	/** Set the interaction context for an object, which must implement the UDemoInteractionContextInterface. */
	UFUNCTION(BlueprintCallable)
	static void SetInteractionContext(UObject* Object, const FDemoInteractionContext& Context);
};
