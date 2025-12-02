// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/AbilityEquipment.h"

#include "AbilitySystemGlobals.h"
#include "GameItemsModule.h"
#include "Equipment/AbilityEquipmentDef.h"
#include "Equipment/GameEquipmentComponent.h"
#include "Equipment/GameEquipmentDef.h"
#include "Misc/DataValidation.h"


#define LOCTEXT_NAMESPACE "GameplayAbilityItemsEditor"

UAbilityEquipment::UAbilityEquipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUseLevelStat(false)
{
}

void UAbilityEquipment::OnEquipped()
{
	Super::OnEquipped();

	GiveAbilitySets();
}

void UAbilityEquipment::OnUnequipped()
{
	Super::OnUnequipped();

	RemoveAbilitySets();
}

int32 UAbilityEquipment::GetAbilityLevel() const
{
	return (bUseLevelStat && LevelStatTag.IsValid()) ? EquipmentSpec.TagStats.GetStackCount(LevelStatTag) : -1;
}

void UAbilityEquipment::GiveAbilitySets()
{
	// grant abilities on server only
	if (!GetOwningActor()->HasAuthority())
	{
		return;
	}

	const UAbilityEquipmentDef* AbilityEquipDef = GetEquipmentDefCDO<UAbilityEquipmentDef>();
	if (!AbilityEquipDef)
	{
		UE_LOG(LogGameItems, Error, TEXT("%s[%s] %s must be a UAbilityEquipmentDef"),
			*GetOwner()->GetDebugPrefix(), *GetReadableName(), *GetEquipmentDef()->GetName());
		return;
	}

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningActor());
	if (!AbilitySystem)
	{
		UE_LOG(LogGameItems, Error, TEXT("%s[%s] No AbilitySystem found"),
			*GetOwner()->GetDebugPrefix(), *GetReadableName());
		return;
	}

	UE_LOG(LogGameItems, Verbose, TEXT("%s[%s] Giving %d ability sets"),
		*GetOwner()->GetDebugPrefix(), *GetReadableName(), AbilityEquipDef->AbilitySets.Num());

	for (const UExtendedAbilitySet* AbilitySet : AbilityEquipDef->AbilitySets)
	{
		if (!AbilitySet)
		{
			UE_LOG(LogGameItems, Error, TEXT("%s[%s] %s has invalid AbilitySet"),
				*GetOwner()->GetDebugPrefix(), *GetReadableName(), *GetNameSafe(AbilityEquipDef));
			continue;
		}

		AbilitySetHandles = AbilitySet->GiveToAbilitySystem(AbilitySystem, this, GetAbilityLevel());
	}
}

void UAbilityEquipment::RemoveAbilitySets()
{
	if (!GetOwningActor() || !GetOwningActor()->HasAuthority())
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningActor()))
	{
		if (AbilitySetHandles.AbilitySet)
		{
			AbilitySetHandles.AbilitySet->RemoveFromAbilitySystem(AbilitySystem, AbilitySetHandles, bEndImmediately, bKeepAttributeSets);
		}
	}
}

#if WITH_EDITOR
EDataValidationResult UAbilityEquipment::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (!LevelStatTag.IsValid())
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(LOCTEXT("LevelStatTagNotSet", "LevelStatTag is not set"));
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
