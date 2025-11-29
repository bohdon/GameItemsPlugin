// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerComponentInterface.h"
#include "GameItemContainerInterface.h"
#include "GameFramework/PlayerState.h"
#include "ItemsDemoPlayerState.generated.h"

class UGameItemContainerComponent;
class ULocalPlayerSaveGame;


/**
 * PlayerState for the project. Stores the game item containers for the player.
 */
UCLASS()
class GAMEITEMSPLUGIN_API AItemsDemoPlayerState
	: public APlayerState,
	  public IGameItemContainerInterface,
	  public IGameItemContainerComponentInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UGameItemContainerComponent> GameItemContainerComponent;

public:
	AItemsDemoPlayerState(const FObjectInitializer& ObjectInitializer);

	virtual void ClientInitialize(AController* Controller) override;

	// IGameItemContainerInterface
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const override;
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const override;

	// IGameItemContainerComponentInterface
	virtual UGameItemContainerComponent* GetItemContainerComponent() const override;

	void CommitSaveGame(ULocalPlayerSaveGame* SaveGame) const;
	void LoadSaveGame(ULocalPlayerSaveGame* SaveGame);
};
