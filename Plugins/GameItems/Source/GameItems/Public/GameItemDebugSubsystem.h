// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameItemDebugSubsystem.generated.h"

class UCheatManagerExtension;
class UGameItemDef;


/**
 * Utils and exec functions for debugging game items.
 */
UCLASS(Config = Game)
class GAMEITEMS_API UGameItemDebugSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void AddItemCheatsExtension(APlayerController* Player);

	UFUNCTION(BlueprintCallable)
	void RemoveItemCheatsExtension(APlayerController* Player);

protected:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UCheatManagerExtension>> SpawnedExtensions;

	virtual void OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer);
};
