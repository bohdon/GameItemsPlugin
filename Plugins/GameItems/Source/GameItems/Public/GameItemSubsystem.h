// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameItemSubsystem.generated.h"

class UGameItemContainerComponent;
class AHUD;
class UCanvas;
class UGameItem;
class UGameItemDef;
class UGameItemFragment;


/**
 * Subsystem for working with game items.
 */
UCLASS()
class GAMEITEMS_API UGameItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Create and return a new game item from definition. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	UGameItem* CreateGameItem(UObject* Outer, TSubclassOf<UGameItemDef> ItemDef, int32 Count);

	/** Duplicate and return a new game item. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	UGameItem* DuplicateGameItem(UObject* Outer, UGameItem* Item);

	/**
	 * Find a return an item fragment by class.
	 * Convenience function that uses the GameItemSubsystem.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Meta = (DeterminesOutputType = "FragmentClass"), Category = "GameItems")
	const UGameItemFragment* FindFragment(TSubclassOf<UGameItemDef> ItemDef, TSubclassOf<UGameItemFragment> FragmentClass) const;

	/** Return all game item containers from an actor. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	TArray<UGameItemContainerComponent*> GetAllContainers(AActor* Actor) const;

	/** Find a return a game item container from an actor by tag.*/
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	UGameItemContainerComponent* FindContainerByTag(AActor* Actor, FGameplayTag IdTag) const;

protected:
	void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
};
