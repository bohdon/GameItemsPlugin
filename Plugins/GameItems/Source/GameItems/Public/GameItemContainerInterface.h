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
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const = 0;

	/** Return an item container by tag id. */
	virtual UGameItemContainer* GetItemContainer(FGameplayTag IdTag) const;

	/** Return all available item container ids. */
	virtual TArray<FGameplayTag> GetAllItemContainerIds() const;
};
