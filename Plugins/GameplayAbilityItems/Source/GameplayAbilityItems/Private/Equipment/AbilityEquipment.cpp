// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/AbilityEquipment.h"

#include "AbilitySystemGlobals.h"
#include "GameItem.h"
#include "GameItemsModule.h"
#include "Equipment/AbilityEquipmentDef.h"
#include "Equipment/GameEquipmentDef.h"


UAbilityEquipment::UAbilityEquipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUseItemLevel(false)
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

int32 UAbilityEquipment::GetItemLevel() const
{
	if (!ItemLevelTag.IsValid())
	{
		return -1;
	}

	const UGameItem* Item = Cast<UGameItem>(Instigator);
	if (!Item)
	{
		return -1;
	}

	return Item->GetTagStat(ItemLevelTag);
}

void UAbilityEquipment::GiveAbilitySets()
{
#if WITH_SERVER_CODE
	// grant abilities on server only
	if (!GetOwningActor()->HasAuthority())
	{
		return;
	}

	const UAbilityEquipmentDef* AbilityEquipDef = GetEquipmentDefCDO<UAbilityEquipmentDef>();
	if (!AbilityEquipDef)
	{
		UE_LOG(LogGameItems, Error, TEXT("[%s] %s must be a UAbilityEquipmentDef"),
		       *GetReadableName(), *GetEquipmentDef()->GetName());
		return;
	}

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningActor());
	if (!AbilitySystem)
	{
		UE_LOG(LogGameItems, Error, TEXT("[%s] No AbilitySystem found"),
		       *GetReadableName());
		return;
	}

	for (const UExtendedAbilitySet* AbilitySet : AbilityEquipDef->AbilitySets)
	{
		if (!AbilitySet)
		{
			UE_LOG(LogGameItems, Error, TEXT("[%s] %s has invalid AbilitySet"),
			       *GetReadableName(), *GetNameSafe(AbilityEquipDef));
			continue;
		}

		AbilitySetHandles = AbilitySet->GiveToAbilitySystem(AbilitySystem, Instigator, bUseItemLevel ? GetItemLevel() : -1);
	}
#endif
}

void UAbilityEquipment::RemoveAbilitySets()
{
#if WITH_SERVER_CODE
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
#endif
}
