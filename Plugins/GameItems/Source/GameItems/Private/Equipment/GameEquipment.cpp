// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameEquipment.h"

#include "Equipment/GameEquipmentDef.h"
#include "Net/UnrealNetwork.h"


UGameEquipment::UGameEquipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameEquipment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentDef);
	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}

UWorld* UGameEquipment::GetWorld() const
{
	if (const AActor* Owner = GetOwner())
	{
		return Owner->GetWorld();
	}
	return nullptr;
}

void UGameEquipment::SetEquipmentDef(TSubclassOf<UGameEquipmentDef> InEquipmentDef)
{
	EquipmentDef = InEquipmentDef;
}

const UGameEquipmentDef* UGameEquipment::GetEquipmentDefCDO() const
{
	return GetDefault<UGameEquipmentDef>(EquipmentDef);
}

AActor* UGameEquipment::GetOwner() const
{
	return Cast<AActor>(GetOuter());
}

AActor* UGameEquipment::GetSpawnedActorOfClass(TSubclassOf<AActor> ActorClass) const
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor->IsA(ActorClass))
		{
			return Actor;
		}
	}
	return nullptr;
}

void UGameEquipment::SpawnEquipmentActors()
{
}

void UGameEquipment::DestroyEquipmentActors()
{
}

void UGameEquipment::OnEquipped()
{
	OnEquipped_BP();
}

void UGameEquipment::OnUnequipped()
{
	OnUnequipped_BP();
}

void UGameEquipment::OnRep_Instigator()
{
}
