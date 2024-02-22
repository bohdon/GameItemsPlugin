// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameItemCollectionInterface.generated.h"

class UGameItem;


UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint))
class UGameItemCollectionInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for an object that manages a collection of game item containers.
 * Used to aggregate counts and also to generalize 'ownership' of an item across containers.
 */
class GAMEITEMS_API IGameItemCollectionInterface
{
	GENERATED_BODY()

public:
	/** Return the total quantity of an item in the collection. */
	virtual int32 GetTotalMatchingItemCount(const UGameItem* Item) const = 0;
};
