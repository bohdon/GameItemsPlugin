﻿// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerComponent.h"
#include "GameItemTypes.h"
#include "GameplayTagContainer.h"
#include "WorldConditionQuery.h"
#include "DropTable/GameItemDropTableRow.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameItemStatics.generated.h"

class UGameItem;
class UGameItemContainer;
class UGameItemDef;
class UGameItemFragment;
class UGameItemSubsystem;


/**
 * Static functions for working with game items.
 */
UCLASS()
class GAMEITEMS_API UGameItemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Return the GameItemContainerComponent for an actor, using IGameItemContainerComponentInterface where possible. */
	UFUNCTION(BlueprintPure, Category = "GameItems")
	static UGameItemContainerComponent* GetItemContainerComponentForActor(AActor* Actor);

	/** Return all game item containers from an actor. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	static TArray<UGameItemContainer*> GetAllItemContainersForActor(AActor* Actor);

	/** Return a game item container from an actor. */
	UFUNCTION(BlueprintCallable, Category = "GameItems", meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	static UGameItemContainer* GetItemContainerForActor(AActor* Actor, FGameplayTag ContainerId);

	/** Find and return an item fragment by class. */
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "FragmentClass"), Category = "GameItems")
	static const UGameItemFragment* FindGameItemFragment(const UObject* WorldContextObject, TSubclassOf<UGameItemDef> ItemDef,
	                                                     TSubclassOf<UGameItemFragment> FragmentClass);

	/** Return an item container by id from an array of containers. */
	UFUNCTION(BlueprintCallable, Category = "GameItems", meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	static UGameItemContainer* GetItemContainerById(const TArray<UGameItemContainer*>& Containers, FGameplayTag ContainerId);

	UFUNCTION(BlueprintCallable)
	static bool IsEquipmentConditionMet(UGameItem* Item);

	UFUNCTION(BlueprintCallable)
	static bool IsDropConditionMet(TSubclassOf<UGameItemDef> ItemDef, AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "GameItems")
	static void SelectItemsFromDropTableRow(const FGameItemDropContext& Context, const FGameItemDropTableRow& DropTableRow,
	                                        TArray<FGameItemDefStack>& OutItems);

	/** Return a random index for an array, given an array (of matching size) of relative probabilities, or -1 if the array is empty. */
	UFUNCTION(BlueprintPure, Category="GameItems|Utilities")
	static int32 GetWeightedRandomArrayIndex(const TArray<float>& Probabilities);

	static bool EvaluateWorldCondition(const UObject* Owner, const FWorldConditionQueryDefinition& Condition, const FWorldConditionContextData& ContextData);
};
