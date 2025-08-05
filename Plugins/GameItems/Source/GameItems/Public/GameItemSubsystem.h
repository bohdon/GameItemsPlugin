﻿// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DisplayDebugHelpers.h"
#include "GameItemTypes.h"
#include "GameplayTagContainer.h"
#include "DropTable/GameItemDropContext.h"
#include "Engine/DataTable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameItemSubsystem.generated.h"

class AHUD;
class IGameItemContainerInterface;
class UCanvas;
class UGameItem;
class UGameItemContainer;
class UGameItemContainerComponent;
class UGameItemDef;
class UGameItemFragment;


/**
 * Subsystem for working with game items.
 */
UCLASS(Config = Game)
class GAMEITEMS_API UGameItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Create and return a new game item from definition. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	UGameItem* CreateItem(UObject* Outer, TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/**
	 * Create a new game item and add it to a container.
	 * @return The newly created items, which may have been split depending on the container rules.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	TArray<UGameItem*> CreateItemInContainer(UGameItemContainer* Container, TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/** Duplicate and return a new game item. If count is > 0, set a new count for the item, otherwise use the original item count. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	UGameItem* DuplicateItem(UObject* Outer, UGameItem* Item, int32 Count = -1);

	/**
	 * Split a game item and return a new item with part of the original quantity.
	 * The split item will not be added to any container. Will return null if the item cannot be split.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	UGameItem* SplitItem(UObject* Outer, UGameItem* Item, int32 Count = 1);

	/**
	 * Move an item from one container to another. If bAllowPartial is true, allow moving only some
	 * of the item if the target container can't receive the full amount.
	 * @return The item or items that were moved into the target container.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	TArray<UGameItem*> MoveItem(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer,
	                            UGameItem* Item, int32 TargetSlot = -1, bool bAllowPartial = true);

	/**
	 * Move multiple items from one container to another. If bAllowPartial is true, allow moving only some
	 * of each item if the target container can't receive the full amount.
	 * @return The item or items that were moved into the target container.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	TArray<UGameItem*> MoveItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, TArray<UGameItem*> Items, bool bAllowPartial = true);

	/**
	 * Move all items from one container to another. If bAllowPartial is true, allow moving only some
	 * of each item if the target container can't receive the full amount.
	 * @return The item or items that were moved into the target container.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	TArray<UGameItem*> MoveAllItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, bool bAllowPartial = true);

	/**
	 * Select items from a drop table.
	 * @return The item definitions and quantities that were selected.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	TArray<FGameItemDefStack> SelectItemsFromDropTable(const FGameItemDropContext& Context, FDataTableRowHandle DropTableEntry);

	/**
	 * Select and create new game items from a drop table.
	 * @return The newly created game items.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	TArray<UGameItem*> CreateItemsFromDropTable(UObject* Outer, const FGameItemDropContext& Context, FDataTableRowHandle DropTableEntry);

	/**
	 * Find a return an item fragment by class.
	 * Convenience function that uses the GameItemSubsystem.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Meta = (DeterminesOutputType = "FragmentClass"), Category = "GameItems")
	const UGameItemFragment* FindFragment(TSubclassOf<UGameItemDef> ItemDef, TSubclassOf<UGameItemFragment> FragmentClass) const;

	/** Return a game item container component from an actor, using the IGameItemContainerComponentInterface if possible. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	UGameItemContainerComponent* GetContainerComponentForActor(const AActor* Actor) const;

	/** Return all game item containers from an actor, using the IGameItemContainerInterface if possible. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	TArray<UGameItemContainer*> GetAllContainersForActor(const AActor* Actor) const;

	/** Return a game item container from an actor, using the IGameItemContainerInterface if possible. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems", meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	UGameItemContainer* GetContainerForActor(const AActor* Actor, FGameplayTag ContainerId) const;

	/** Return the default container from an actor, using the IGameItemContainerInterface if possible. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	UGameItemContainer* GetDefaultContainerForActor(const AActor* Actor) const;

	/** Find a game item container interface from an actor. Attempts to cast the actor first, then search for a component. */
	virtual const IGameItemContainerInterface* GetContainerInterfaceForActor(const AActor* Actor) const;

protected:
	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

public:
	/** Return the game item subsystem given a world context object. */
	static UGameItemSubsystem* GetGameItemSubsystem(const UObject* WorldContextObject);
};
