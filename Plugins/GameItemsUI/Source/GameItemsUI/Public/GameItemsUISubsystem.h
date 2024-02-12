// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainer.h"
#include "GameItemContainerProvider.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameItemsUISubsystem.generated.h"

class UGameItem;
class UGameItemContainer;
class UGameItemContainerProvider;
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

	/** Retrieve a game item from an object, supports several game item view models, or a game item itself. */
	UFUNCTION(BlueprintPure, Category = "GameItems|UI")
	UGameItem* GetItemFromObject(UObject* ItemObject) const;

	/** Retrieve a game item and container from a container slot view model. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems|UI")
	void GetContainerAndItem(UObject* ViewModelObject, bool& bSuccess, UGameItemContainer*& Container, UGameItem*& Item) const;

	/** Retrieve a game item container from a provider class, given view context. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems|UI")
	UGameItemContainer* GetContainerFromProvider(TSubclassOf<UGameItemContainerProvider> Provider,
	                                             const FGameplayTag& ContainerId, const FGameItemViewContext& Context);

protected:
	/** All container view models that have been created. */
	UPROPERTY(Transient)
	TArray<UGameItemContainerViewModel*> ContainerViewModels;

	UGameItemContainerViewModel* CreateContainerViewModel(UGameItemContainer* Container);
};
