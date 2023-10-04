// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/AbilityEquipmentDef.h"

#include "Equipment/AbilityEquipment.h"


UAbilityEquipmentDef::UAbilityEquipmentDef(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EquipmentClass = UAbilityEquipment::StaticClass();
}
