// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SaveGame.h"
#include "ItemsDemoGameMode.generated.h"


/**
 * Game mode for the project.
 */
UCLASS()
class GAMEITEMSPLUGIN_API AItemsDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AItemsDemoGameMode(const FObjectInitializer& ObjectInitializer);

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void OnCommitSaveGame(USaveGame* SaveGame);
};
