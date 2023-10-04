// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/GameEquipmentDef.h"

#include "AbilityEquipmentDef.generated.h"

class UExtendedAbilitySet;


/**
 * An equipment definition that grants ability sets when equipped.
 */
UCLASS()
class GAMEPLAYABILITYITEMS_API UAbilityEquipmentDef : public UGameEquipmentDef
{
	GENERATED_BODY()

public:
	UAbilityEquipmentDef(const FObjectInitializer& ObjectInitializer);

	/** The ability sets to grant when equipped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<TObjectPtr<const UExtendedAbilitySet>> AbilitySets;
};
