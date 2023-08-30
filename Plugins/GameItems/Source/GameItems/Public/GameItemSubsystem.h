// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameItemSubsystem.generated.h"

class UGameItem;
class UGameItemDef;


/**
 * 
 */
UCLASS()
class GAMEITEMS_API UGameItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Create and return a new game item from definition. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	UGameItem* CreateGameItem(UObject* Outer, TSubclassOf<UGameItemDef> ItemDef, int32 Count);
};
