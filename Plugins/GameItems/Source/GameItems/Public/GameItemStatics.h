﻿// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameItemStatics.generated.h"

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
	static UGameItemSubsystem* GetItemSubsystemFromContextObject(const UObject* Object);

	/** Return all game item containers component from an actor, using the UGameItemSubsystem. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	static TArray<UGameItemContainer*> GetAllGameItemContainersFromActor(AActor* Actor);

	/** Return a game item container component from an actor, using the UGameItemSubsystem. */
	UFUNCTION(BlueprintPure, Category = "GameItems")
	static UGameItemContainer* GetGameItemContainerFromActor(AActor* Actor, FGameplayTag ContainerId);

	/** Find and return an item fragment by class. */
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "FragmentClass"), Category = "GameItems")
	static const UGameItemFragment* FindGameItemFragment(const UObject* WorldContextObject, TSubclassOf<UGameItemDef> ItemDef,
	                                                     TSubclassOf<UGameItemFragment> FragmentClass);
};
