// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/AbilityEquipment.h"

#include "AbilitySystemGlobals.h"
#include "GameItem.h"
#include "GameItemsModule.h"
#include "Equipment/AbilityEquipmentDef.h"


UAbilityEquipment::UAbilityEquipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  bUseItemLevel(false)
{
}

void UAbilityEquipment::OnEquipped()
{
	GiveAbilitySets();

	Super::OnEquipped();
}

void UAbilityEquipment::OnUnequipped()
{
	RemoveAbilitySets();

	Super::OnUnequipped();
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
	const UAbilityEquipmentDef* AbilityEquipDef = GetEquipmentDefCDO<UAbilityEquipmentDef>();
	UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());

	if (AbilityEquipDef && AbilitySystem)
	{
		for (const UExtendedAbilitySet* AbilitySet : AbilityEquipDef->AbilitySets)
		{
			if (!AbilitySet)
			{
				UE_LOG(LogGameItems, Error, TEXT("%s has invalid AbilitySet"), *GetNameSafe(AbilityEquipDef));
				continue;
			}

			AbilitySetHandles = AbilitySet->GiveToAbilitySystem(AbilitySystem, Instigator, bUseItemLevel ? GetItemLevel() : -1);
		}
	}
}

void UAbilityEquipment::RemoveAbilitySets()
{
	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		AbilitySetHandles.AbilitySet->RemoveFromAbilitySystem(AbilitySystem, AbilitySetHandles);
	}
}
