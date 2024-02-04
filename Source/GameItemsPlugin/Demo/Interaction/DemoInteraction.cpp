// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoInteraction.h"

#include "DemoInteractionComponent.h"
#include "DemoInteractionContextInterface.h"


FDemoInteractionContext::FDemoInteractionContext(APawn* InInteractor, UDemoInteractionComponent* InInteractionComponent)
	: Interactor(InInteractor),
	  InteractionComponent(InInteractionComponent)
{
}

UDemoInteraction::UDemoInteraction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UDemoInteraction::Trigger_Implementation(const FDemoInteractionContext& Context)
{
	return true;
}

void UDemoInteraction::SetInteractionContext(UObject* Object, const FDemoInteractionContext& Context)
{
	if (Object && Object->Implements<UDemoInteractionContextInterface>())
	{
		IDemoInteractionContextInterface::Execute_SetInteractionContext(Object, Context);
	}
}
