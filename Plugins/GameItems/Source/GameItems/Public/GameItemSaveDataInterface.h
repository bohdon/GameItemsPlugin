// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "UObject/Interface.h"
#include "GameItemSaveDataInterface.generated.h"


UINTERFACE(MinimalAPI, Meta = (CannotImplementInterfaceInBlueprint))
class UGameItemSaveDataInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for a save game object that can store game item save data.
 */
class GAMEITEMS_API IGameItemSaveDataInterface
{
	GENERATED_BODY()

public:
	/** Return the player and world game item save data. */
	virtual FPlayerAndWorldGameItemSaveData& GetItemSaveData() = 0;
};
