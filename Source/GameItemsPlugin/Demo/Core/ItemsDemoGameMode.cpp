// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoGameMode.h"

#include "Demo/Player/ItemsDemoCharacter.h"
#include "Demo/Player/ItemsDemoHUD.h"
#include "Demo/Player/ItemsDemoPlayerController.h"
#include "Demo/Player/ItemsDemoPlayerState.h"
#include "Demo/SaveGame/DemoPlayerSaveSubsystem.h"
#include "Engine/LocalPlayer.h"


AItemsDemoGameMode::AItemsDemoGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = AItemsDemoPlayerController::StaticClass();
	PlayerStateClass = AItemsDemoPlayerState::StaticClass();
	HUDClass = AItemsDemoHUD::StaticClass();
	DefaultPawnClass = AItemsDemoCharacter::StaticClass();
}
