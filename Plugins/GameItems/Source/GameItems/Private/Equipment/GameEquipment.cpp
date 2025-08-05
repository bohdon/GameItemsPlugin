// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameEquipment.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Equipment/GameEquipmentComponent.h"
#include "Equipment/GameEquipmentDef.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
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
	if (const AActor* Owner = GetOwningActor())
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

UGameEquipmentComponent* UGameEquipment::GetOwner() const
{
	return GetTypedOuter<UGameEquipmentComponent>();
}

AActor* UGameEquipment::GetOwningActor() const
{
	return GetTypedOuter<AActor>();
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
	const UGameEquipmentDef* EquipmentCDO = GetEquipmentDefCDO();
	if (!EquipmentCDO)
	{
		return;
	}

	USceneComponent* AttachTarget = GetTargetAttachComponent();
	check(AttachTarget);

	AActor* OwningActor = AttachTarget->GetOwner();

	for (const FGameEquipmentActorSpawnInfo& SpawnInfo : EquipmentCDO->ActorsToSpawn)
	{
		if (!SpawnInfo.ActorClass)
		{
			continue;
		}

		AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorClass, FTransform::Identity, OwningActor);
		NewActor->FinishSpawning(FTransform::Identity, true);
		NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
		NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

		SpawnedActors.Add(NewActor);
	}
}

void UGameEquipment::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}

	SpawnedActors.Empty();
}

void UGameEquipment::OnEquipped()
{
	SpawnEquipmentActors();

	OnEquipped_BP();
}

void UGameEquipment::OnUnequipped()
{
	DestroyEquipmentActors();

	OnUnequipped_BP();
}

USceneComponent* UGameEquipment::GetTargetAttachComponent() const
{
	AActor* OwningActor = GetOwningActor();
	if (!OwningActor)
	{
		return nullptr;
	}

	if (const ACharacter* Char = Cast<ACharacter>(OwningActor))
	{
		return Char->GetMesh();
	}
	return OwningActor->GetRootComponent();
}

void UGameEquipment::OnRep_Instigator()
{
}
