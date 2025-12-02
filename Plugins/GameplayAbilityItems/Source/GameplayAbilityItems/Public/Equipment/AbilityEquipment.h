// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExtendedAbilitySet.h"
#include "GameplayTagContainer.h"
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

	/** If true, apply abilities and effects at a level defined by the tag stats of this equipment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseLevelStat;

	/**
	 * If set, apply abilities/effects at a level defined by this equipment's tag stats.
	 * If granted by game items, the tag stats are copied from the item.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bUseLevelStat", GameplayTagFilter = "GameItemStatTagsCategory"))
	FGameplayTag LevelStatTag;

	/** End abilities immediately when this equipment is removed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEndImmediately = true;

	/** Keep attribute sets when this equipment is removed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bKeepAttributeSets = false;

	/** Return the level of the abilities/effects to apply. */
	virtual int32 GetAbilityLevel() const;

protected:
	/** Ability and effect handles that were granted by this equipment. */
	UPROPERTY()
	FExtendedAbilitySetHandles AbilitySetHandles;

	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;

	virtual void GiveAbilitySets();
	virtual void RemoveAbilitySets();

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
