// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "GameItemContainerInterface.generated.h"

class UGameItemContainerComponent;


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
	/** Return all item containers for this actor. */
	virtual TArray<UGameItemContainerComponent*> GetAllItemContainerComponent() const = 0;

	/** Return the item container to use for this actor, by id tag. */
	virtual UGameItemContainerComponent* GetItemContainerComponent(FGameplayTag IdTag) const;
};
