// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameItemStatics.generated.h"

class UGameItemContainerComponent;
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
	static UGameItemSubsystem* GetItemSubsystemFromContextObject(const UObject* Object);

	/** Return all game item container components from an actor. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	static TArray<UGameItemContainerComponent*> GetAllGameItemContainersFromActor(AActor* Actor);

	/**
	 * Return a game item container component from an actor, using the IGameItemContainerInterface where possible.
	 * @param Actor The actor to retrieve an item container from.
	 * @param IdTag The id tag of the item container to retrieve.
	 */
	UFUNCTION(BlueprintPure, Category = "GameItems")
	static UGameItemContainerComponent* GetGameItemContainerFromActor(AActor* Actor, FGameplayTag IdTag);

	/** Find and return an item fragment by class. */
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "FragmentClass"), Category = "GameItems")
	static const UGameItemFragment* FindGameItemFragment(const UObject* WorldContextObject, TSubclassOf<UGameItemDef> ItemDef,
	                                                     TSubclassOf<UGameItemFragment> FragmentClass);
};
