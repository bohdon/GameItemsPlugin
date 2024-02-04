// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoGameMode.h"

#include "Demo/Player/ItemsDemoHUD.h"
#include "Demo/Player/ItemsDemoPlayerController.h"


AItemsDemoGameMode::AItemsDemoGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = AItemsDemoPlayerController::StaticClass();
	HUDClass = AItemsDemoHUD::StaticClass();
}
