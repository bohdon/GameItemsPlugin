// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoPlayerSaveSubsystem.h"

#include "DemoSaveGame.h"
#include "GameItemsModule.h"
#include "JsonObjectConverter.h"
#include "Demo/Player/ItemsDemoPlayerState.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"


UDemoPlayerSaveSubsystem::UDemoPlayerSaveSubsystem()
	: SaveGameClass(UDemoSaveGame::StaticClass()),
	  SaveSlotName(TEXT("DemoSave")),
	  bSaveOnStopPIE(true),
	  bIsSavingDisabled(false)
{
}

void UDemoPlayerSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
#if WITH_EDITOR
	FWorldDelegates::OnPIEEnded.AddUObject(this, &ThisClass::OnPIEEnded);
#endif
}

void UDemoPlayerSaveSubsystem::Deinitialize()
{
	FWorldDelegates::OnPIEEnded.RemoveAll(this);
}

void UDemoPlayerSaveSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
#if WITH_EDITOR
	if (!SaveGame)
	{
		// load automatically in editor (for easy debug play)
		// in a real game you'd likely load in front-end
		LoadOrCreateSaveGame();
	}
#endif
}

#if WITH_EDITOR
void UDemoPlayerSaveSubsystem::OnPIEEnded(UGameInstance* GameInstance)
{
	if (bSaveOnStopPIE)
	{
		WriteSaveGame();
	}
}
#endif

bool UDemoPlayerSaveSubsystem::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return HasAnyFlags(RF_ClassDefaultObject) ? false : ProcessConsoleExec(Cmd, Ar, nullptr);
}

void UDemoPlayerSaveSubsystem::SetSavingDisabled(bool bDisabled)
{
	bIsSavingDisabled = bDisabled;
}

void UDemoPlayerSaveSubsystem::LoadOrCreateSaveGame()
{
	SaveGame = ULocalPlayerSaveGame::LoadOrCreateSaveGameForLocalPlayer(SaveGameClass, GetOuterULocalPlayer(), SaveSlotName);

	if (SaveGame)
	{
		if (const APlayerController* PlayerController = GetOuterULocalPlayer()->GetPlayerController(nullptr))
		{
			if (AItemsDemoPlayerState* PlayerState = PlayerController->GetPlayerState<AItemsDemoPlayerState>())
			{
				PlayerState->LoadSaveGame(SaveGame);
			}
		}
	}
}

void UDemoPlayerSaveSubsystem::CommitSaveGame()
{
	if (!SaveGame)
	{
		return;
	}

	OnCommitSaveGameEvent.Broadcast(SaveGame);

	// commit player state
	if (const APlayerController* PlayerController = GetOuterULocalPlayer()->GetPlayerController(nullptr))
	{
		if (const AItemsDemoPlayerState* PlayerState = Cast<AItemsDemoPlayerState>(PlayerController->PlayerState))
		{
			PlayerState->CommitSaveGame(SaveGame);
		}
	}
}

void UDemoPlayerSaveSubsystem::WriteSaveGame(bool bCommit)
{
	if (bIsSavingDisabled || !SaveGame)
	{
		return;
	}

	if (bCommit)
	{
		CommitSaveGame();
	}
	SaveGame->SaveGameToSlotForLocalPlayer();
}

void UDemoPlayerSaveSubsystem::DeleteSaveGame()
{
	UGameplayStatics::DeleteGameInSlot(SaveSlotName, GetOuterULocalPlayer()->GetPlatformUserIndex());
}

void UDemoPlayerSaveSubsystem::DumpSaveGame()
{
	if (const UDemoSaveGame* DemoSave = Cast<UDemoSaveGame>(SaveGame))
	{
		FString SaveGameJson;
		FJsonObjectConverter::UStructToJsonObjectString<UDemoSaveGame>(*DemoSave, SaveGameJson);

		UE_LOG(LogGameItems, Log, TEXT("[GameItemsDemo] Save data:\n%s"), *SaveGameJson);
	}
}
