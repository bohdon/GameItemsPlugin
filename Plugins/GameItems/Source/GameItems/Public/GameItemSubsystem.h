// Copyright Bohdon Sayre, All Rights Reserved.

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
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	UGameItem* CreateItem(UObject* Outer, TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/** Create and return a new game item from save data. */
	UGameItem* CreateItemFromSaveData(UObject* Outer, const FGameItemSaveData& ItemSaveData);

	/** Create a new game item and add it to a container. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	void CreateItemInContainer(UGameItemContainer* Container, TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1, bool bWarn = true);

	/** Return true if a container has all items at all indicated quantities. Useful for calculating costs. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	bool HasItemStacks(UGameItemContainer* Container, TArray<FGameItemDefStack> ItemStacks) const;

	/** Remove items from a container by item defs and counts. Useful for subtracting item costs. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	bool RemoveItemStacks(UGameItemContainer* Container, TArray<FGameItemDefStack> ItemStacks, bool bAllowPartial = false) const;

	/** Create and return a duplicate of a game item. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	UGameItem* DuplicateItem(UObject* Outer, UGameItem* Item);

	/**
	 * Split a game item and return a new item with part of the original quantity.
	 * The split item will not be added to any container. Will return null if the item cannot be split.
	 * @param Outer The outer of the split off item.
	 * @param Item The item to be split.
	 * @param Count The quantity to remove from the original item. Must be less than the items original count.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	UGameItem* SplitItem(UObject* Outer, UGameItem* Item, int32 Count = 1);

	/**
	 * Move an item from one container to another.
	 * If bAllowPartial is true, allow moving only some of the item if the target container can't receive the full amount.
	 * Handles potentially sending items via RPC if the net ownership of the containers differs.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	void MoveItem(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, UGameItem* Item, int32 TargetSlot = -1, bool bAllowPartial = true);

	/**
	 * Move multiple items from one container to another.
	 * If bAllowPartial is true, allow moving only some of each item if the target container can't receive the full amount.
	 * Handles potentially sending items via RPC if the net ownership of the containers differs.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	void MoveItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, TArray<UGameItem*> Items, bool bAllowPartial = true);

	/**
	 * Move all items from one container to another. If bAllowPartial is true, allow moving only some
	 * of each item if the target container can't receive the full amount.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	void MoveAllItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, bool bAllowPartial = true);

	/**
	 * Select items from a drop table.
	 * @return The item definitions and quantities that were selected.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	TArray<FGameItemDefStack> SelectItemsFromDropTable(const FGameItemDropContext& Context, FDataTableRowHandle DropTableEntry);

	/**
	 * Select and create new game items from a drop table.
	 * @return The newly created game items.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	TArray<UGameItem*> CreateItemsFromDropTable(UObject* Outer, const FGameItemDropContext& Context, FDataTableRowHandle DropTableEntry);

	/**
	 * Find a return an item fragment by class.
	 * Convenience function that uses the GameItemSubsystem.
	 */
	UE_DEPRECATED(5.7, "FindFragment via subsystem is deprecated, use UGameItemStatics")
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Meta = (DeterminesOutputType = "FragmentClass", DeprecatedFunction), Category = "GameItems")
	const UGameItemFragment* FindFragment(TSubclassOf<UGameItemDef> ItemDef, TSubclassOf<UGameItemFragment> FragmentClass) const;

	template <class T>
	UE_DEPRECATED(5.7, "FindFragment via subsystem is deprecated, use UGameItemStatics")
	const T* FindFragment(TSubclassOf<UGameItemDef> ItemDef) const
	{
		static_assert(TIsDerivedFrom<T, UGameItemFragment>::IsDerived, TEXT("T must derive from UGameItemFragment"));
		return Cast<T>(FindFragment(ItemDef, T::StaticClass()));
	}

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
	/**
	 * Check for potential network move items between containers (from local-only to server or vice versa)
	 * and handle using RPCs as needed, potentially serializing and recreating items.
	 * @return True if the move will be handled via network RPC calls, false if it should be done normally.
	 */
	bool HandleNetMoveItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, const TArray<FGameItemMove>& Moves);

	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);

public:
	/** Return the game item subsystem given a world context object. */
	static UGameItemSubsystem* GetGameItemSubsystem(const UObject* WorldContextObject);
};
