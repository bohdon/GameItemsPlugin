// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameItemsUISubsystem.generated.h"

class UGameItemContainer;
class UGameItemContainerViewModel;


/**
 * Subsystem for working with game items UI.
 */
UCLASS()
class GAMEITEMSUI_API UGameItemsUISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Get a view model for an item container, reusing an existing one if it already exists. */
	UFUNCTION(BlueprintCallable, Category = "GameItems|UI")
	UGameItemContainerViewModel* GetOrCreateContainerViewModel(UGameItemContainer* Container);

	/** Get a view model for an item container, reusing an existing one if it already exists. */
	UFUNCTION(BlueprintCallable, Category = "GameItems|UI")
	UGameItemContainerViewModel* GetOrCreateContainerViewModelForActor(AActor* Actor, FGameplayTag ContainerId);

protected:
	/** All container view models that have been created. */
	UPROPERTY(Transient)
	TArray<UGameItemContainerViewModel*> ContainerViewModels;

	UGameItemContainerViewModel* CreateContainerViewModel(UGameItemContainer* Container);
};
