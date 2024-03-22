// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemDropParams.h"
#include "InstancedStruct.h"
#include "GameItemDropContext.generated.h"


/**
 * Contextual info that is passed around when selecting items from a drop table.
 * Contains the params and other info.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemDropContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TInstancedStruct<FGameItemDropParams> Params;
};
