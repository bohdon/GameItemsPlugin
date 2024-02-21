// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemSaveDataInterface.h"
#include "GameFramework/SaveGame.h"
#include "DemoSaveGame.generated.h"


enum class EDemoSaveGameVersion : uint32
{
	Initial,

	// new versions should be added above this line
	VersionPlusOne,
	LatestVersion = VersionPlusOne - 1
};

/**
 * Save game for the demo project.
 */
UCLASS()
class GAMEITEMSPLUGIN_API UDemoSaveGame : public USaveGame,
                                          public IGameItemSaveDataInterface
{
	GENERATED_BODY()

public:
	UDemoSaveGame();
	
	/** The version when the data was saved */
	UPROPERTY(SaveGame)
	uint32 SavedDataVersion;

	/** Save data for all game items and containers of both the player and world. */
	UPROPERTY(SaveGame)
	FPlayerAndWorldGameItemSaveData ItemSaveData;

	// IGameItemSaveDataInterface
	virtual FPlayerAndWorldGameItemSaveData& GetItemSaveData() override { return ItemSaveData; }

	virtual void Serialize(FArchive& Ar) override;
};
