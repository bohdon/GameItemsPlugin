// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DemoInteraction.generated.h"

class UDemoInteractionComponent;
class UInputAction;


USTRUCT(BlueprintType)
struct FDemoInteractionContext
{
	GENERATED_BODY()

	FDemoInteractionContext()
	{
	}

	FDemoInteractionContext(APawn* InInteractor, UDemoInteractionComponent* InInteractionComponent);

	/** The pawn that triggered the interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<APawn> Interactor;

	/** The interaction component which owns the interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UDemoInteractionComponent> InteractionComponent;
};


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
