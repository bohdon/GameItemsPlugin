// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoGameMode.h"

#include "Demo/Player/ItemsDemoCharacter.h"
#include "Demo/Player/ItemsDemoHUD.h"
#include "Demo/Player/ItemsDemoPlayerController.h"
#include "Demo/Player/ItemsDemoPlayerState.h"
#include "Demo/SaveGame/DemoSaveGameSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"


AItemsDemoGameMode::AItemsDemoGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = AItemsDemoPlayerController::StaticClass();
	PlayerStateClass = AItemsDemoPlayerState::StaticClass();
	HUDClass = AItemsDemoHUD::StaticClass();
	DefaultPawnClass = AItemsDemoCharacter::StaticClass();
}

void AItemsDemoGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UDemoSaveGameSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UDemoSaveGameSubsystem>();
	SaveSubsystem->OnCommitSaveGameEvent.AddUObject(this, &AItemsDemoGameMode::OnCommitSaveGame);
}

void AItemsDemoGameMode::Logout(AController* Exiting)
{
	if (const APlayerController* PlayerController = Cast<APlayerController>(Exiting))
	{
		const UDemoSaveGameSubsystem* SaveSubsystem = UGameInstance::GetSubsystem<UDemoSaveGameSubsystem>(GetGameInstance());
		USaveGame* SaveGame = SaveSubsystem->GetSaveGame();
		AItemsDemoPlayerState* PlayerState = PlayerController->GetPlayerState<AItemsDemoPlayerState>();
		if (PlayerState && SaveGame)
		{
			PlayerState->CommitSaveGame(SaveGame);
		}
	}

	Super::Logout(Exiting);
}

void AItemsDemoGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UDemoSaveGameSubsystem* SaveSubsystem = UGameInstance::GetSubsystem<UDemoSaveGameSubsystem>(GetGameInstance()))
	{
		SaveSubsystem->OnCommitSaveGameEvent.RemoveAll(this);
	}
}

void AItemsDemoGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// load player state save game
	const UDemoSaveGameSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UDemoSaveGameSubsystem>();
	if (USaveGame* SaveGame = SaveSubsystem->GetSaveGame())
	{
		AItemsDemoPlayerState* PlayerState = NewPlayer->GetPlayerState<AItemsDemoPlayerState>();
		PlayerState->LoadSaveGame(SaveGame);
	}

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void AItemsDemoGameMode::OnCommitSaveGame(USaveGame* SaveGame)
{
	// commit for all players
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (const APlayerController* PlayerController = Iterator->Get())
		{
			AItemsDemoPlayerState* PlayerState = PlayerController->GetPlayerState<AItemsDemoPlayerState>();
			PlayerState->CommitSaveGame(SaveGame);
		}
	}
}
