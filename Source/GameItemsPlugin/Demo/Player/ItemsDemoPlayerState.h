// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerComponentInterface.h"
#include "GameItemContainerInterface.h"
#include "Demo/SaveGame/DemoSaveGameSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SaveGame.h"
#include "ItemsDemoPlayerState.generated.h"

class UGameItemContainerComponent;


/**
 * PlayerState for the project. Stores the game item containers for the player.
 */
UCLASS()
class GAMEITEMSPLUGIN_API AItemsDemoPlayerState : public APlayerState,
                                                  public IGameItemContainerInterface,
                                                  public IGameItemContainerComponentInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	UGameItemContainerComponent* GameItemContainerComponent;

public:
	AItemsDemoPlayerState(const FObjectInitializer& ObjectInitializer);

	// IGameItemContainerInterface
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const override;
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const override;

	// IGameItemContainerComponentInterface
	virtual UGameItemContainerComponent* GetItemContainerComponent() const override;

	void CommitSaveGame(USaveGame* SaveGame);

	void LoadSaveGame(USaveGame* SaveGame);
};
