// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameEquipmentTypes.h"
#include "GameItemFragment.h"
#include "WorldConditionQuery.h"
#include "GameItemFragment_Equipment.generated.h"

class UGameEquipmentDef;


/**
 * Item fragment that defines equipment to grant from this item.
 */
UCLASS(DisplayName = "Equipment")
class GAMEITEMS_API UGameItemFragment_Equipment : public UGameItemFragment
{
	GENERATED_BODY()

public:
	UGameItemFragment_Equipment(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSubclassOf<UGameEquipmentDef> EquipmentDef;

	/** Conditions that must be met for the equipment to be applied. */
	UPROPERTY(EditAnywhere, Category = "Equipment")
	FWorldConditionQueryDefinition Condition;

	/**
	 * Create the equipment spec to use for applying the equipment of this fragment.
	 * Copies in item tag stats or other info as needed.
	 */
	virtual FGameEquipmentSpec CreateEquipmentSpecForItem(UGameItem* Item) const;
};
