// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DemoInteractionTypes.generated.h"

class APawn;
class UDemoInteraction;
class UDemoInteractionComponent;


/**
 * Context for the currently active interaction.
 */
USTRUCT(BlueprintType)
struct FDemoInteractionContext
{
	GENERATED_BODY()

	FDemoInteractionContext()
	{
	}

	FDemoInteractionContext(APawn* InInteractor, UDemoInteractionComponent* InInteractionComponent, UDemoInteraction* InInteraction);

	/** The pawn that triggered the interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<APawn> Interactor;

	/** The interaction component which owns the interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UDemoInteractionComponent> InteractionComponent;

	/** The interaction being triggered. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UDemoInteraction> Interaction;
};
