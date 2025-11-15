// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"
#include "GameItemContainerInterface.generated.h"

class UGameItemContainer;
class UGameItemDef;


UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint))
class UGameItemContainerInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for an object that provides one or more GameItemContainers
 */
class GAMEITEMS_API IGameItemContainerInterface
{
	GENERATED_BODY()

public:
	/** Return all available item containers. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const = 0;

	/** Return all available item container ids. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	virtual TArray<FGameplayTag> GetAllItemContainerIds() const;

	/** Return an item container by tag id. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"), Category = "GameItems")
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const;

	/** Return the default item container. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	virtual UGameItemContainer* GetDefaultItemContainer() const;

	/** Return default container that should hold a specific item. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	virtual UGameItemContainer* GetDefaultContainerForItem(TSubclassOf<UGameItemDef> ItemDef) const;
};
