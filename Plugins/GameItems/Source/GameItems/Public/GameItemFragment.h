// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameItemFragment.generated.h"

class UGameItem;


/**
 * A fragment of an item definition.
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class GAMEITEMS_API UGameItemFragment : public UObject
{
	GENERATED_BODY()

public:
	/** Called when a game item instance has been created. */
	virtual void OnItemCreated(UGameItem* Item) const;
};
