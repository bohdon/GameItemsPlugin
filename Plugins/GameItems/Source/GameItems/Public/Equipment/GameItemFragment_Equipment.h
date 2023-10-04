// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "GameItemFragment_Equipment.generated.h"

class UGameEquipmentDef;


/**
 * Item fragment that defines equipment to grant from this item.
 */
UCLASS()
class GAMEITEMS_API UGameItemFragment_Equipment : public UGameItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSubclassOf<UGameEquipmentDef> EquipmentDef;
};
