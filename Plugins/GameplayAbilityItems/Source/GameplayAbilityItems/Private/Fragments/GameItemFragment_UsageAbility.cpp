// Copyright Bohdon Sayre, All Rights Reserved.

#include "Fragments/GameItemFragment_UsageAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"


bool UGameItemFragment_UsageAbility::Use_Implementation(UGameItem* Item, AActor* Instigator) const
{
	bool bActivated = false;
	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Instigator))
	{
		if (AbilityTags.IsValid())
		{
			bActivated |= AbilitySystem->TryActivateAbilitiesByTag(AbilityTags);
		}
		if (Ability)
		{
			bActivated |= AbilitySystem->TryActivateAbilityByClass(Ability);
		}
	}
	return bActivated;
}
