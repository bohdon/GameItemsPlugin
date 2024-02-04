// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "GameItemContainerInterface.generated.h"

class UGameItemContainer;


UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint))
class UGameItemContainerInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for an object that provides one or more GameItemContainerComponents
 */
class GAMEITEMS_API IGameItemContainerInterface
{
	GENERATED_BODY()

public:
	/** Return all available item containers. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const = 0;

	/** Return an item container by tag id. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const;

	/** Return all available item container ids. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	virtual TArray<FGameplayTag> GetAllItemContainerIds() const;
};
