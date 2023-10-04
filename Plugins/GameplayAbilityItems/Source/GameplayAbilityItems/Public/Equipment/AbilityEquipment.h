// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExtendedAbilitySet.h"
#include "Equipment/GameEquipment.h"
#include "AbilityEquipment.generated.h"


/**
 * Equipment that grants ability sets.
 */
UCLASS()
class GAMEPLAYABILITYITEMS_API UAbilityEquipment : public UGameEquipment
{
	GENERATED_BODY()

public:
	UAbilityEquipment(const FObjectInitializer& ObjectInitializer);

	/** If true, use the level of the item that granted this equipment as the level for granted abilities and effects. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseItemStatLevel;

	/** The item stat tag to use as the ability level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bUseItemStatLevel"))
	FGameplayTag ItemLevelTag;

	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;

	/** Return the level of the item that instigated this equipment. */
	virtual int32 GetItemLevel() const;

protected:
	/** Ability and effect handles that were granted by this equipment. */
	UPROPERTY()
	FExtendedAbilitySetHandles AbilitySetHandles;

	virtual void GiveAbilitySets();
	virtual void RemoveAbilitySets();
};
