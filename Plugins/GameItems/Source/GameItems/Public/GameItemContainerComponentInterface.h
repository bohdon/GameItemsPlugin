// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameItemContainerComponentInterface.generated.h"

class UGameItemContainerComponent;


UINTERFACE(Meta = (CannotImplementInterfaceInBlueprint))
class UGameItemContainerComponentInterface : public UInterface
{
	GENERATED_BODY()
};


/**
 * Interface for an actor that provides a UGameItemContainerComponent
 */
class GAMEITEMS_API IGameItemContainerComponentInterface
{
	GENERATED_BODY()

public:
	/** Return the UGameItemContainerComponent for this actor. */
	virtual UGameItemContainerComponent* GetItemContainerComponent() const = 0;
};
