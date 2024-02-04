// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoInteraction.h"


UDemoInteraction::UDemoInteraction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UDemoInteraction::Trigger_Implementation(APawn* Interactor)
{
	return true;
}
