// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameItemContainerStockRules.generated.h"

class UGameItem;
class UGameItemContainerComponent;


/**
 * Defines rules that restrict what and how many items can exist in a container.
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class GAMEITEMS_API UGameItemContainerStockRules : public UObject
{
	GENERATED_BODY()

public:
	virtual bool CanAddItemToContainer(UGameItemContainerComponent* Container, UGameItem* Item) { return false; }
	virtual int32 GetItemMaxCount(UGameItemContainerComponent* Container, UGameItem* Item) { return 0; }
	virtual int32 GetItemStackMaxCount(UGameItemContainerComponent* Container, UGameItem* Item) { return 0; }
};
