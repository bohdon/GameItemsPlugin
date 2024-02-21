// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoGameInstance.h"

#include "Demo/SaveGame/DemoSaveGame.h"
#include "Demo/SaveGame/DemoSaveGameSubsystem.h"

void UItemsDemoGameInstance::Init()
{
	Super::Init();

	UDemoSaveGameSubsystem* SaveSubsystem = GetSubsystem<UDemoSaveGameSubsystem>();
	SaveSubsystem->SaveSlotName = TEXT("DemoSave");
	SaveSubsystem->SaveGameClass = UDemoSaveGame::StaticClass();

	if (!SaveSubsystem->LoadSaveGame())
	{
		SaveSubsystem->CreateSaveGame();
	}
}
