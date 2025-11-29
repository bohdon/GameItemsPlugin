// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Templates/SubclassOf.h"
#include "DemoPlayerSaveSubsystem.generated.h"

class ULocalPlayerSaveGame;


/**
 * Demo subsystem for saving and loading items and other game state.
 */
UCLASS()
class GAMEITEMSPLUGIN_API UDemoPlayerSaveSubsystem
	: public ULocalPlayerSubsystem,
	  public FSelfRegisteringExec
{
	GENERATED_BODY()

public:
	UDemoPlayerSaveSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

#if WITH_EDITOR
	void OnPIEEnded(UGameInstance* GameInstance);
#endif

	// FSelfRegisteringExec
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	/** Set whether saving is enabled. Does not prevent committing save game data, only prevents writing it to disk. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void SetSavingDisabled(bool bDisabled);

	/** Return the current save game. */
	UFUNCTION(BlueprintPure, Category = "Save")
	ULocalPlayerSaveGame* GetSaveGame() const { return SaveGame; }

	/** Load the save game, or create a new one. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void LoadOrCreateSaveGame();

	/**
	 * Update the current save game with the latest data from all objects in the world, but don't write
	 * the save game to disk yet. This is also called before WriteSaveGame to ensure data is up to date,
	 * though objects should additionally handle committing save data at the end of their lifecycle.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void CommitSaveGame();

	/** Commit and write the current save game to disk. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void WriteSaveGame(bool bCommit = true);

	/** Delete the save game from disk. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void DeleteSaveGame();

	/** Log the current save game as json. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void DumpSaveGame();

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FCommitSaveGameDelegate, ULocalPlayerSaveGame* /*SaveGame*/);

	/** Called when gameplay state should be committed to the save game. */
	FCommitSaveGameDelegate OnCommitSaveGameEvent;

public:
	/** The save game class to create. */
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<ULocalPlayerSaveGame> SaveGameClass;

	UPROPERTY(BlueprintReadWrite)
	FString SaveSlotName;

#if WITH_EDITORONLY_DATA
	/** Write the save game when stopping play in editor. */
	UPROPERTY(BlueprintReadWrite)
	bool bSaveOnStopPIE;
#endif

	/** Disable writing of the save game to disk. */
	UPROPERTY(BlueprintReadWrite)
	bool bIsSavingDisabled;

protected:
	/** The current save game. */
	UPROPERTY()
	TObjectPtr<ULocalPlayerSaveGame> SaveGame;
};
