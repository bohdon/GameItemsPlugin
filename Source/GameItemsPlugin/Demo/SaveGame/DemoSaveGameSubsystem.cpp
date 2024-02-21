// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoSaveGameSubsystem.h"

#include "GameItemSaveDataInterface.h"
#include "JsonObjectConverter.h"
#include "Engine/World.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"


UDemoSaveGameSubsystem::UDemoSaveGameSubsystem()
	: SaveUserIndex(0),
	  bSaveOnStopPIE(true),
	  bIsSavingDisabled(false)
{
}

void UDemoSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

void UDemoSaveGameSubsystem::Deinitialize()
{
#if WITH_EDITOR
	if (bSaveOnStopPIE)
	{
		const UWorld* World = GetWorld();
		if (World && World->WorldType == EWorldType::PIE)
		{
			WriteSaveGame();
		}
	}
#endif
}

bool UDemoSaveGameSubsystem::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return HasAnyFlags(RF_ClassDefaultObject) ? false : ProcessConsoleExec(Cmd, Ar, nullptr);
}

void UDemoSaveGameSubsystem::SetSavingDisabled(bool bDisabled)
{
	bIsSavingDisabled = bDisabled;
}

void UDemoSaveGameSubsystem::CommitSaveGame()
{
	if (SaveGame)
	{
		OnCommitSaveGameEvent.Broadcast(SaveGame);
	}
}

void UDemoSaveGameSubsystem::WriteSaveGame()
{
	if (bIsSavingDisabled || !SaveGame)
	{
		return;
	}

	CommitSaveGame();
	UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, SaveUserIndex);
}

bool UDemoSaveGameSubsystem::LoadSaveGame()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		return false;
	}

	SaveGame = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex);
	return SaveGame != nullptr;
}

void UDemoSaveGameSubsystem::CreateSaveGame()
{
	SaveGame = UGameplayStatics::CreateSaveGameObject(SaveGameClass);
}

void UDemoSaveGameSubsystem::DeleteSaveGame()
{
	UGameplayStatics::DeleteGameInSlot(SaveSlotName, SaveUserIndex);
}

void UDemoSaveGameSubsystem::DumpSaveGame()
{
	IGameItemSaveDataInterface* ItemSaveDataInterface = Cast<IGameItemSaveDataInterface>(SaveGame);
	if (!ItemSaveDataInterface)
	{
		return;
	}

	const FPlayerAndWorldGameItemSaveData& ItemData = ItemSaveDataInterface->GetItemSaveData();

	FString SaveGameJson;
	FJsonObjectConverter::UStructToJsonObjectString<FPlayerAndWorldGameItemSaveData>(ItemData, SaveGameJson);

	UE_LOG(LogTemp, Log, TEXT("Save data:\n%s"), *SaveGameJson);
}
