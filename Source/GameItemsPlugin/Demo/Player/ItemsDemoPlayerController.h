﻿// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "ItemsDemoPlayerController.generated.h"

class UCommonActivatableWidgetStack;


/**
 * Player controller class for the project.
 */
UCLASS()
class GAMEITEMSPLUGIN_API AItemsDemoPlayerController : public APlayerController,
                                                       public IGameItemContainerInterface
{
	GENERATED_BODY()

public:
	/** Return a UI layer by tag. */
	UFUNCTION(BlueprintPure, Meta = (GameplayTagFilter = "UI.Layer"))
	UCommonActivatableWidgetStack* GetUILayer(FGameplayTag LayerTag) const;

	// IGameItemContainerInterface
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const override;
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const override;
};
