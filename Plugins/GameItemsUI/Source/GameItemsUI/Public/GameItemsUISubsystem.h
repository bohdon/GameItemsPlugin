// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainer.h"
#include "GameItemContainerProvider.h"
#include "GameplayTagContainer.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameItemsUISubsystem.generated.h"

class UGameItem;
class UGameItemContainer;
class UGameItemContainerProvider;
class UGameItemControllerComponent;
class UVM_GameItemContainer;
class UVM_GameItemContainerTransfer;
class UVM_GameItemSlot;


/**
 * Local player subsystem for working with game items UI and operations.
 *
 * Provides utils that use view models directly for convenience,
 * and routes operations to the UGameItemControllerComponent of the player
 * for full network support, including client control of server-owned containers.
 */
UCLASS()
class GAMEITEMSUI_API UGameItemsUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	static UGameItemsUISubsystem* GetFromPlayerController(const APlayerController* Player);
	static UGameItemsUISubsystem* GetFromUserWidget(const UUserWidget* UserWidget);

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	UFUNCTION(BlueprintPure, Category = "GameItems|UI")
	APlayerController* GetPlayerController() const { return PlayerController; }

	UFUNCTION(BlueprintPure, Category = "GameItems|UI")
	virtual UGameItemControllerComponent* GetGameItemController() const;

	/** Move the item in a slot to a new container. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Move Item (by VM)")
	virtual void MoveItem(UVM_GameItemSlot* FromSlot, UGameItemContainer* To, bool bAllowPartial = true) const;

	/** Move an item from one slot to another, swapping or stacking as needed. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, DisplayName = "Move Swap or Stack Item (by VMs)")
	virtual void MoveSwapOrStackItem(UVM_GameItemSlot* FromSlot, UVM_GameItemSlot* ToSlot, bool bAllowPartial = true) const;

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

	/** Create a view model with utils for common item transfer operations between two containers. */
	UFUNCTION(BlueprintCallable, Category = "GameItems|UI", Meta = (DefaultToSelf = "Outer"))
	virtual UVM_GameItemContainerTransfer* CreateTransferViewModel(UObject* Outer);

	/** Retrieve a game item from an object, supports several game item view models, or a game item itself. */
	UFUNCTION(BlueprintPure, Category = "GameItems|UI")
	virtual UGameItem* GetItemFromObject(UObject* ItemObject) const;

	/** Retrieve a game item and container from a container slot view model. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems|UI")
	virtual void GetContainerAndItem(UObject* ViewModelObject, bool& bSuccess, UGameItemContainer*& Container, UGameItem*& Item) const;

	/** Retrieve a game item container from a provider class, given view context. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems|UI", meta = (GameplayTagFilter = "GameItemContainerIdTagsCategory"))
	UGameItemContainer* GetContainerFromProvider(
		TSubclassOf<UGameItemContainerProvider> Provider,
		const FGameplayTag& ContainerId,
		const FGameItemViewContext& Context);

protected:
	/** The current player controller. */
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PlayerController;

	/** All container view models that have been created. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UVM_GameItemContainer>> ContainerViewModels;

	UVM_GameItemContainer* CreateContainerViewModel(UGameItemContainer* Container);
};
