// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "GameItemDefBlueprint.generated.h"


UCLASS(BlueprintType)
class GAMEITEMS_API UGameItemDefBlueprint : public UBlueprint
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual bool SupportedByDefaultBlueprintFactory() const override { return false; }
#endif
};
