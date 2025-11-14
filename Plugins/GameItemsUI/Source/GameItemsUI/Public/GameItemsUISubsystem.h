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
class UVM_GameItemContainer;
class UVM_GameItemSlot;


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
	virtual UVM_GameItemContainer* GetOrCreateContainerViewModel(UGameItemContainer* Container);

	/** Get a view model for an item container, reusing an existing one if it already exists. */
	UFUNCTION(BlueprintCallable, Category = "GameItems|UI", meta = (GameplayTagFilter = "GameItemContainerIdTagsCategory"))
	virtual UVM_GameItemContainer* GetOrCreateContainerViewModelForActor(AActor* Actor, FGameplayTag ContainerId);

	/** Create a slot view model a slot in a container. */
	UFUNCTION(BlueprintCallable, Category = "GameItems|UI", Meta = (DefaultToSelf = "Outer"))
	virtual UVM_GameItemSlot* CreateSlotViewModelForContainer(UObject* Outer, UGameItemContainer* InContainer, int32 InSlot);

	/** Retrieve a game item from an object, supports several game item view models, or a game item itself. */
	UFUNCTION(BlueprintPure, Category = "GameItems|UI")
	virtual UGameItem* GetItemFromObject(UObject* ItemObject) const;

	/** Retrieve a game item and container from a container slot view model. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems|UI")
	virtual void GetContainerAndItem(UObject* ViewModelObject, bool& bSuccess, UGameItemContainer*& Container, UGameItem*& Item) const;

	/** Retrieve a game item container from a provider class, given view context. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems|UI", meta = (GameplayTagFilter = "GameItemContainerIdTagsCategory"))
	UGameItemContainer* GetContainerFromProvider(TSubclassOf<UGameItemContainerProvider> Provider,
	                                             const FGameplayTag& ContainerId, const FGameItemViewContext& Context);

	/** Move an item from one slot to another, swapping or stacking as needed. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void MoveSwapOrStackItem(UVM_GameItemSlot* FromSlot, UVM_GameItemSlot* ToSlot, bool bAllowPartial = true) const;

protected:
	/** All container view models that have been created. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UVM_GameItemContainer>> ContainerViewModels;

	UVM_GameItemContainer* CreateContainerViewModel(UGameItemContainer* Container);
};
