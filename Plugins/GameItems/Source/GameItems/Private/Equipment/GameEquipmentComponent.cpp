// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameEquipmentComponent.h"

#include "GameItemsModule.h"
#include "GameItemStatics.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "Equipment/GameEquipment.h"
#include "Equipment/GameEquipmentDef.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"


UGameEquipmentComponent::UGameEquipmentComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	bAutoActivate = true;
	SetIsReplicatedByDefault(true);
}

void UGameEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, EquipmentList, Params);
}

void UGameEquipmentComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner() && !GetOwner()->HasAnyFlags(RF_ClassDefaultObject))
	{
		EquipmentList.OnPreReplicatedRemoveEvent.AddUObject(this, &ThisClass::OnPreReplicatedRemove);
		EquipmentList.OnPostReplicatedAddEvent.AddUObject(this, &ThisClass::OnPostReplicatedAdd);
		EquipmentList.OnPostReplicatedChangeEvent.AddUObject(this, &ThisClass::OnPostReplicatedChange);
	}
}

void UGameEquipmentComponent::UninitializeComponent()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		RemoveAllEquipment();
	}

	Super::UninitializeComponent();
}

void UGameEquipmentComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// register any existing equipment
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FGameEquipmentListEntry& Entry : EquipmentList.GetEntries())
		{
			if (IsValid(Entry.Equipment))
			{
				AddReplicatedSubObject(Entry.Equipment);
			}
		}
	}
}

void UGameEquipmentComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	
	if (IsActive())
	{
		for (UGameEquipment* Equipment : GetAllEquipment())
		{
			Equipment->Equip();
		}
	}
}

void UGameEquipmentComponent::Deactivate()
{
	Super::Deactivate();

	// don't destroy equipment, just deactivate it
	for (UGameEquipment* Equipment : GetAllEquipment())
	{
		Equipment->Unequip();
	}
}

void UGameEquipmentComponent::OnRep_IsActive()
{
	Super::OnRep_IsActive();

	for (UGameEquipment* Equipment : GetAllEquipment())
	{
		if (IsActive())
		{
			Equipment->Equip();
		}
		else
		{
			Equipment->Unequip();
		}
	}
}

UGameEquipment* UGameEquipmentComponent::ApplyEquipment(TSubclassOf<UGameEquipmentDef> EquipmentDef, UObject* Instigator)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return nullptr;
	}

	if (!EquipmentDef)
	{
		return nullptr;
	}

	const UGameEquipmentDef* EquipmentDefCDO = GetDefault<UGameEquipmentDef>(EquipmentDef);
	if (EquipmentDefCDO->EquipmentClass == nullptr)
	{
		UE_LOG(LogGameItems, Error, TEXT("%s Cant apply equipment, %s.EquipmentClass is not set"),
		       *GetDebugPrefix(), *EquipmentDef->GetName());
		return nullptr;
	}

	UGameEquipment* NewEquipment = NewObject<UGameEquipment>(this, EquipmentDefCDO->EquipmentClass);
	NewEquipment->SetEquipmentDef(EquipmentDef);
	NewEquipment->SetInstigator(Instigator);

	if (IsActive())
	{
		NewEquipment->Equip();
	}

	EquipmentList.AddEntry(NewEquipment);

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(NewEquipment);
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s Applied equipment: %s (Instigator: %s)"),
	       *GetDebugPrefix(), *NewEquipment->GetName(), *GetNameSafe(Instigator));

	return NewEquipment;
#else
	return nullptr;
#endif
}

void UGameEquipmentComponent::RemoveEquipment(UGameEquipment* Equipment)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	if (!Equipment)
	{
		return;
	}

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		RemoveReplicatedSubObject(Equipment);
	}

	Equipment->Unequip();

	EquipmentList.RemoveEntry(Equipment);

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s Removed equipment: %s"),
	       *GetDebugPrefix(), *Equipment->GetName());
#endif
}

void UGameEquipmentComponent::RemoveAllEquipment()
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	TArray<UGameEquipment*> AllEquipment = GetAllEquipment();
	for (UGameEquipment* Equipment : AllEquipment)
	{
		RemoveEquipment(Equipment);
	}
#endif
}

UGameEquipment* UGameEquipmentComponent::FindEquipment(TSubclassOf<UGameEquipment> EquipmentClass) const
{
	for (const FGameEquipmentListEntry& Entry : EquipmentList.GetEntries())
	{
		UGameEquipment* Equipment = Entry.Equipment;
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
	for (const FGameEquipmentListEntry& Entry : EquipmentList.GetEntries())
	{
		UGameEquipment* Equipment = Entry.Equipment;
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
	for (const auto& Entry : EquipmentList.GetEntries())
	{
		UGameEquipment* Equipment = Entry.Equipment;
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
	for (const auto& Entry : EquipmentList.GetEntries())
	{
		UGameEquipment* Equipment = Entry.Equipment;
		if (IsValid(Equipment))
		{
			Result.Add(Equipment);
		}
	}
	return Result;
}

void UGameEquipmentComponent::OnPreReplicatedRemove(FGameEquipmentListEntry& Entry)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] %s"),
	       *GetDebugPrefix(), __func__, *Entry.GetDebugString());

	if (Entry.Equipment)
	{
		if (IsActive())
		{
			Entry.Equipment->Unequip();
		}

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			RemoveReplicatedSubObject(Entry.Equipment);
		}
	}
}

void UGameEquipmentComponent::OnPostReplicatedAdd(FGameEquipmentListEntry& Entry)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] %s"),
	       *GetDebugPrefix(), __func__, *Entry.GetDebugString());

	if (Entry.Equipment)
	{
		if (IsActive())
		{
			Entry.Equipment->Equip();
		}

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			AddReplicatedSubObject(Entry.Equipment);
		}
	}
}

void UGameEquipmentComponent::OnPostReplicatedChange(FGameEquipmentListEntry& Entry)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] %s"),
	       *GetDebugPrefix(), __func__, *Entry.GetDebugString());

	// this should only be called if the entry went from null -> valid, so treat it like OnPostReplicatedAdd
	if (Entry.Equipment)
	{
		if (IsActive())
		{
			Entry.Equipment->Equip();
		}

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			AddReplicatedSubObject(Entry.Equipment);
		}
	}
}

FString UGameEquipmentComponent::GetDebugPrefix() const
{
	return FString::Printf(TEXT("%s[%s]"), *UGameItemStatics::GetNetDebugPrefix(this), *GetReadableName());
}
