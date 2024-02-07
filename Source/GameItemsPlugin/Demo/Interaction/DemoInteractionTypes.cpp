// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoInteractionTypes.h"

#include "DemoInteraction.h"
#include "DemoInteractionComponent.h"
#include "GameFramework/Pawn.h"


FDemoInteractionContext::FDemoInteractionContext(APawn* InInteractor, UDemoInteractionComponent* InInteractionComponent, UDemoInteraction* InInteraction)
	: Interactor(InInteractor),
	  InteractionComponent(InInteractionComponent),
	  Interaction(InInteraction)
{
}
