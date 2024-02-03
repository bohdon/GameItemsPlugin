// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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
};
