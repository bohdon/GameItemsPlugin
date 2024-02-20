// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "GameItemCheatsExtension.generated.h"

class UGameItemDef;


UCLASS()
class UGameItemCheatsExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	/** Give a new item to an actor. If no actor is given, gives the item to the player pawn. */
	UFUNCTION(Exec)
	void GiveItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1, AActor* Actor = nullptr);
};
