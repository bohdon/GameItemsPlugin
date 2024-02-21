// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/SubclassOf.h"
#include "DemoSaveGameSubsystem.generated.h"

class USaveGame;


/**
 * Demo subsystem for saving and loading items and other game state.
 */
UCLASS()
class GAMEITEMSPLUGIN_API UDemoSaveGameSubsystem : public UGameInstanceSubsystem,
                                                   public FSelfRegisteringExec
{
	GENERATED_BODY()

public:
	UDemoSaveGameSubsystem();

	/** The save game class to create. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<USaveGame> SaveGameClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SaveUserIndex;

	/** Write the save game when stopping play in editor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveOnStopPIE;

	/** Disable writing of the save game to disk. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSavingDisabled;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FSelfRegisteringExec
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	/** Set whether saving is enabled. Does not prevent committing save game data, only prevents writing it to disk. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void SetSavingDisabled(bool bDisabled);

	/** Return the current save game. */
	UFUNCTION(BlueprintPure, Category = "Save")
	USaveGame* GetSaveGame() const { return SaveGame; }

	/**
	 * Update the current save game with the latest data from all objects in the world, but don't write
	 * the save game to disk yet. This is also called before WriteSaveGame to ensure data is up to date,
	 * though objects should additionally handle committing save data at the end of their lifecycle.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void CommitSaveGame();

	/** Commit and write the current save game to disk. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void WriteSaveGame();

	/** Load the save game. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	bool LoadSaveGame();

	/** Create a new save game. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void CreateSaveGame();

	/** Delete the save game from disk. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void DeleteSaveGame();

	/** Log the current save game as json. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Save")
	void DumpSaveGame();

	DECLARE_MULTICAST_DELEGATE_OneParam(FCommitSaveGameDelegate, USaveGame* /*SaveGame*/);

	/** Called when gameplay state should be committed to the save game. */
	FCommitSaveGameDelegate OnCommitSaveGameEvent;

protected:
	/** The current save game. */
	UPROPERTY()
	TObjectPtr<USaveGame> SaveGame;
};
