// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameEquipmentComponent.h"

#include "GameItemsModule.h"
#include "Engine/ActorChannel.h"
#include "Equipment/GameEquipment.h"
#include "Equipment/GameEquipmentDef.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"


UGameEquipmentComponent::UGameEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
}

void UGameEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}


void UGameEquipmentComponent::UninitializeComponent()
{
	RemoveAllEquipment();

	Super::UninitializeComponent();
}

void UGameEquipmentComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// register any existing equipment
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FGameEquipmentListEntry& Entry : EquipmentList.Entries)
		{
			if (IsValid(Entry.GetEquipment()))
			{
				AddReplicatedSubObject(Entry.GetEquipment());
			}
		}
	}
}

bool UGameEquipmentComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bDidWrite = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const FGameEquipmentListEntry& Entry : EquipmentList.Entries)
	{
		UGameEquipment* Equipment = Entry.GetEquipment();
		if (IsValid(Equipment))
		{
			bDidWrite |= Channel->ReplicateSubobject(Equipment, *Bunch, *RepFlags);
		}
	}

	return bDidWrite;
}

UGameEquipment* UGameEquipmentComponent::ApplyEquipment(TSubclassOf<UGameEquipmentDef> EquipmentDef, UObject* Instigator)
{
	if (!EquipmentDef)
	{
		return nullptr;
	}

	const UGameEquipmentDef* EquipmentDefCDO = GetDefault<UGameEquipmentDef>(EquipmentDef);
	if (EquipmentDefCDO->EquipmentClass == nullptr)
	{
		UE_LOG(LogGameItems, Error, TEXT("Cant apply equipment, %s.EquipmentClass is not set"), *EquipmentDef->GetName());
		return nullptr;
	}

	UGameEquipment* NewEquipment = NewObject<UGameEquipment>(this, EquipmentDefCDO->EquipmentClass);
	NewEquipment->SetEquipmentDef(EquipmentDef);
	NewEquipment->SetInstigator(Instigator);
	NewEquipment->OnEquipped();

	EquipmentList.AddEntry(NewEquipment);

	UE_LOG(LogGameItems, Verbose, TEXT("Applied equipment: %s (Instigator: %s)"), *NewEquipment->GetName(), *GetNameSafe(Instigator));

	return NewEquipment;
}

void UGameEquipmentComponent::RemoveEquipment(UGameEquipment* Equipment)
{
	if (!Equipment)
	{
		return;
	}

	if (IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(Equipment);
	}

	Equipment->OnUnequipped();
	Equipment->DestroyEquipmentActors();

	EquipmentList.RemoveEntry(Equipment);

	UE_LOG(LogGameItems, Verbose, TEXT("Removed equipment: %s"), *Equipment->GetName());
}

void UGameEquipmentComponent::RemoveAllEquipment()
{
}

UGameEquipment* UGameEquipmentComponent::FindEquipment(TSubclassOf<UGameEquipment> EquipmentClass) const
{
	for (const FGameEquipmentListEntry& Entry : EquipmentList.Entries)
	{
		UGameEquipment* Equipment = Entry.GetEquipment();
		if (IsValid(Equipment) && Equipment->IsA(EquipmentClass))
		{
			return Equipment;
		}
	}
	return nullptr;
}

TArray<UGameEquipment*> UGameEquipmentComponent::FindAllEquipment(TSubclassOf<UGameEquipment> EquipmentClass) const
{
	TArray<UGameEquipment*> Result;
	for (const FGameEquipmentListEntry& Entry : EquipmentList.Entries)
	{
		UGameEquipment* Equipment = Entry.GetEquipment();
		if (IsValid(Equipment) && Equipment->IsA(EquipmentClass))
		{
			Result.Add(Equipment);
		}
	}
	return Result;
}

TArray<UGameEquipment*> UGameEquipmentComponent::FindAllEquipmentByInstigator(UObject* Instigator) const
{
	TArray<UGameEquipment*> Result;
	for (const auto Entry : EquipmentList.Entries)
	{
		UGameEquipment* Equipment = Entry.GetEquipment();
		if (IsValid(Equipment) && Equipment->GetInstigator() == Instigator)
		{
			Result.Add(Equipment);
		}
	}
	return Result;
}

TArray<UGameEquipment*> UGameEquipmentComponent::GetAllEquipment() const
{
	TArray<UGameEquipment*> Result;
	for (const auto Entry : EquipmentList.Entries)
	{
		UGameEquipment* Equipment = Entry.GetEquipment();
		if (IsValid(Equipment))
		{
			Result.Add(Equipment);
		}
	}
	return Result;
}
