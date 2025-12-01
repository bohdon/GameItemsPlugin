// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameEquipment.h"

#include "GameItemsModule.h"
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

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, EquipmentSpec, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, SpawnedActors, Params);
}

void UGameEquipment::OnRep_SpawnedActors()
{
}

UWorld* UGameEquipment::GetWorld() const
{
	if (const AActor* Owner = GetOwningActor())
	{
		return Owner->GetWorld();
	}
	return nullptr;
}

FString UGameEquipment::GetReadableName() const
{
	return FString::Printf(TEXT("%s(%s)"), *GetNameSafe(EquipmentSpec.EquipmentDef), *GetName());
}

void UGameEquipment::SetEquipmentSpec(const FGameEquipmentSpec& InSpec)
{
	EquipmentSpec = InSpec;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, EquipmentSpec, this);
}

const UGameEquipmentDef* UGameEquipment::GetEquipmentDefCDO() const
{
	return GetDefault<UGameEquipmentDef>(EquipmentSpec.EquipmentDef);
}

UGameEquipmentComponent* UGameEquipment::GetOwner() const
{
	return GetTypedOuter<UGameEquipmentComponent>();
}

AActor* UGameEquipment::GetOwningActor() const
{
	return GetTypedOuter<AActor>();
}

TArray<AActor*> UGameEquipment::GetSpawnedActors() const
{
	TArray<AActor*> Result(SpawnedActors);
	Result.Append(LocalSpawnedActors);
	return Result;
}

AActor* UGameEquipment::GetSpawnedActorOfClass(TSubclassOf<AActor> ActorClass) const
{
	for (AActor* Actor : GetSpawnedActors())
	{
		if (Actor->IsA(ActorClass))
		{
			return Actor;
		}
	}
	return nullptr;
}

void UGameEquipment::Equip()
{
	if (!bIsEquipped)
	{
		bIsEquipped = true;

		UE_LOG(LogGameItems, Verbose, TEXT("%s[%s] Equip"), *GetOwner()->GetDebugPrefix(), *GetReadableName());

		OnEquipped();
		OnEquipped_BP();
	}
}

void UGameEquipment::Unequip()
{
	if (bIsEquipped)
	{
		bIsEquipped = false;

		UE_LOG(LogGameItems, Verbose, TEXT("%s[%s] Unequip"), *GetOwner()->GetDebugPrefix(), *GetReadableName());

		OnUnequipped();
		OnUnequipped_BP();
	}
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

	auto IsLocallyControlled = [](const AActor* Actor)
	{
		if (Actor->GetNetMode() == NM_Client && Actor->GetLocalRole() == ROLE_AutonomousProxy)
		{
			// networked client in control
			return true;
		}
		if (Actor->HasAuthority() && Actor->GetRemoteRole() != ROLE_AutonomousProxy)
		{
			// local authority in control
			return true;
		}
		return false;
	};

	AActor* OwningActor = GetOwningActor();

	for (const FGameEquipmentActorSpawnInfo& SpawnInfo : EquipmentCDO->ActorsToSpawn)
	{
		if (!SpawnInfo.ActorClass)
		{
			continue;
		}

		bool bIsReplicatedSpawn = false;
		if (OwningActor->GetNetMode() != NM_Standalone)
		{
			if (SpawnInfo.NetSpawnPolicy == EGameEquipmentActorSpawnPolicy::ServerInitiated)
			{
				bIsReplicatedSpawn = true;
				if (!OwningActor->HasAuthority())
				{
					// spawn on server only
					continue;
				}
			}
			else if (SpawnInfo.NetSpawnPolicy == EGameEquipmentActorSpawnPolicy::LocalOnly)
			{
				if (!IsLocallyControlled(OwningActor))
				{
					continue;
				}
			}
		}

		UE_LOG(LogGameItems, Verbose, TEXT("%s[%s] Spawning equipment actor: %s (NetSpawnPolicy: %s)"),
			*GetOwner()->GetDebugPrefix(), *GetReadableName(),
			*SpawnInfo.ActorClass->GetName(),
			*UEnum::GetDisplayValueAsText(SpawnInfo.NetSpawnPolicy).ToString());

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = OwningActor;
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(SpawnInfo.ActorClass, SpawnInfo.AttachTransform, SpawnParams);
		NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);
		NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);

		if (bIsReplicatedSpawn)
		{
			// required for the actor to be spawned on clients
			NewActor->SetReplicates(true);

			SpawnedActors.Add(NewActor);
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, SpawnedActors, this);
		}
		else
		{
			LocalSpawnedActors.Add(NewActor);
		}
	}
}

void UGameEquipment::DestroyEquipmentActors()
{
	const AActor* OwningActor = GetOwningActor();
	if (OwningActor->HasAuthority())
	{
		for (AActor* Actor : SpawnedActors)
		{
			if (Actor)
			{
				Actor->Destroy();
			}
		}
		SpawnedActors.Empty();
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, SpawnedActors, this);
	}

	for (AActor* Actor : LocalSpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
	LocalSpawnedActors.Empty();
}

void UGameEquipment::OnEquipped()
{
	SpawnEquipmentActors();
}

void UGameEquipment::OnUnequipped()
{
	DestroyEquipmentActors();
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

	if (USkeletalMeshComponent* FoundSkeletalMesh = OwningActor->FindComponentByClass<USkeletalMeshComponent>())
	{
		return FoundSkeletalMesh;
	}

	return OwningActor->GetRootComponent();
}
