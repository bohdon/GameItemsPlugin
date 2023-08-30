// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameEquipmentTypes.h"
#include "UObject/Object.h"
#include "GameEquipmentDef.generated.h"

class UGameEquipment;


/**
 * Base class for a gameplay equipment definition.
 * Designed to be subclassed in Blueprint for each piece of equipment.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const)
class GAMEITEMS_API UGameEquipmentDef : public UObject
{
	GENERATED_BODY()

public:
	UGameEquipmentDef(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** The equipment class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	TSubclassOf<UGameEquipment> EquipmentClass;

	/** Actors to spawn when equipped. */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	TArray<FGameEquipmentActorSpawnInfo> ActorsToSpawn;
};
