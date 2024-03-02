// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WorldConditionQuery.h"
#include "GameFramework/Actor.h"
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
	/** Return all game item containers from an actor. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	static TArray<UGameItemContainer*> GetAllItemContainersForActor(AActor* Actor);

	/** Return a game item container from an actor. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	static UGameItemContainer* GetItemContainerForActor(AActor* Actor, FGameplayTag ContainerId);

	/** Find and return an item fragment by class. */
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "FragmentClass"), Category = "GameItems")
	static const UGameItemFragment* FindGameItemFragment(const UObject* WorldContextObject, TSubclassOf<UGameItemDef> ItemDef,
	                                                     TSubclassOf<UGameItemFragment> FragmentClass);

	/** Return an item container by id from an array of containers. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	static UGameItemContainer* GetItemContainerById(const TArray<UGameItemContainer*>& Containers, FGameplayTag ContainerId);
	
	UFUNCTION(BlueprintCallable)
	static bool IsEquipmentConditionMet(UGameItem* Item);

	UFUNCTION(BlueprintCallable)
	static bool IsDropConditionMet(TSubclassOf<UGameItemDef> ItemDef, AActor* TargetActor);

protected:
	static bool EvaluateCondition(const UObject* Owner, const FWorldConditionQueryDefinition& Condition, const FWorldConditionContextData& ContextData);
};
