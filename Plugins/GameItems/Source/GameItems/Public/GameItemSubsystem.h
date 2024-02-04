// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DisplayDebugHelpers.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameItemSubsystem.generated.h"

class AHUD;
class IGameItemContainerInterface;
class UCanvas;
class UGameItem;
class UGameItemContainer;
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
	UGameItem* CreateGameItem(UObject* Outer, TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/**
	 * Create a new game item and add it to a container.
	 * @return The newly created items, which may have been split depending on the container rules.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	TArray<UGameItem*> CreateGameItemInContainer(UGameItemContainer* Container, TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/** Duplicate and return a new game item. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	UGameItem* DuplicateGameItem(UObject* Outer, UGameItem* Item);

	/**
	 * Find a return an item fragment by class.
	 * Convenience function that uses the GameItemSubsystem.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Meta = (DeterminesOutputType = "FragmentClass"), Category = "GameItems")
	const UGameItemFragment* FindFragment(TSubclassOf<UGameItemDef> ItemDef, TSubclassOf<UGameItemFragment> FragmentClass) const;

	/** Return all game item containers from an actor, using the IGameItemContainerInterface if possible. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	TArray<UGameItemContainer*> GetAllContainersForActor(AActor* Actor) const;

	/** Return a game item container from an actor, using the IGameItemContainerInterface if possible. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	UGameItemContainer* GetContainerForActor(AActor* Actor, FGameplayTag ContainerId) const;

	/** Return the default container from an actor, using the IGameItemContainerInterface if possible. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	UGameItemContainer* GetDefaultContainerForActor(AActor* Actor) const;

protected:
	/** Return the a game item container interface object, either as the actor or a potential component. */
	virtual IGameItemContainerInterface* GetContainerInterfaceForActor(AActor* Actor) const;

	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
};
