// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoGameMode.h"

#include "ItemsDemoHUD.h"


AItemsDemoGameMode::AItemsDemoGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HUDClass = AItemsDemoHUD::StaticClass();
}
