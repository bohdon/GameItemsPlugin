// Copyright Bohdon Sayre, All Rights Reserved.

#include "Equipment/GameEquipmentDef.h"

#include "Equipment/GameEquipment.h"


UGameEquipmentDef::UGameEquipmentDef(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EquipmentClass = UGameEquipment::StaticClass();
}
