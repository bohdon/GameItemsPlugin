// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ItemsDemoHUD.generated.h"


/**
 * Customized HUD for the demo project.
 */
UCLASS()
class GAMEITEMSPLUGIN_API AItemsDemoHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** Extend the debug actor list to support selecting any actor with gameplay items. */
	virtual void GetDebugActorList(TArray<AActor*>& InOutList) override;
};
