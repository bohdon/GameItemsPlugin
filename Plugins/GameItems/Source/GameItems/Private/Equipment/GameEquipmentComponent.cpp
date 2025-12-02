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

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameEquipmentComponent)


// used in generic "DoAction" functions to:
// - call ServerDoAction if we are local owner (but not authority)
// - execute locally if authority
#define CONDITIONAL_EXECUTE(FuncName, ...) \
	{ \
		bool bExecuteServer; \
		bool bExecuteLocal; \
		GetNetExecutionPlan(bExecuteServer, bExecuteLocal); \
		if (bExecuteServer) \
		{ \
			Server##FuncName(__VA_ARGS__); \
		} \
		if (!bExecuteLocal) \
		{ \
			return; \
		} \
	}


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

void UGameEquipmentComponent::ApplyEquipment(TSubclassOf<UGameEquipmentDef> EquipmentDef)
{
	ApplyEquipmentSpec(FGameEquipmentSpec(EquipmentDef, FGameItemTagStackContainer()));
}

void UGameEquipmentComponent::ApplyEquipmentSpec(const FGameEquipmentSpec& EquipmentSpec)
{
	if (!EquipmentSpec.EquipmentDef)
	{
		return;
	}

	CONDITIONAL_EXECUTE(ApplyEquipmentSpec, EquipmentSpec)

	const UGameEquipmentDef* EquipmentDefCDO = GetDefault<UGameEquipmentDef>(EquipmentSpec.EquipmentDef);
	if (EquipmentDefCDO->EquipmentClass == nullptr)
	{
		UE_LOG(LogGameItems, Error, TEXT("%s Cant apply equipment, %s.EquipmentClass is not set"),
			*GetDebugPrefix(), *EquipmentSpec.EquipmentDef->GetName());
		return;
	}

	UGameEquipment* NewEquipment = NewObject<UGameEquipment>(this, EquipmentDefCDO->EquipmentClass);
	NewEquipment->SetEquipmentSpec(EquipmentSpec);

	EquipmentList.AddEntry(NewEquipment);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, EquipmentList, this);

	OnEquipmentApplied(NewEquipment);
}

void UGameEquipmentComponent::ServerApplyEquipmentSpec_Implementation(const FGameEquipmentSpec& EquipmentSpec)
{
	ApplyEquipmentSpec(EquipmentSpec);
}

void UGameEquipmentComponent::RemoveEquipmentByDef(TSubclassOf<UGameEquipmentDef> EquipmentDef)
{
	if (!EquipmentDef)
	{
		return;
	}

	CONDITIONAL_EXECUTE(RemoveEquipmentByDef, EquipmentDef)

	if (UGameEquipment* Equipment = FindEquipmentByDef(EquipmentDef))
	{
		RemoveEquipment(Equipment);
	}
}

void UGameEquipmentComponent::ServerRemoveEquipmentByDef_Implementation(TSubclassOf<UGameEquipmentDef> EquipmentDef)
{
	RemoveEquipmentByDef(EquipmentDef);
}

void UGameEquipmentComponent::RemoveEquipment(UGameEquipment* Equipment)
{
	if (!Equipment)
	{
		return;
	}

	EquipmentList.RemoveEntry(Equipment);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, EquipmentList, this);

	OnEquipmentRemoved(Equipment);
}

void UGameEquipmentComponent::OnEquipmentApplied(UGameEquipment* Equipment)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s Applied equipment: %s (%s)"),
		*GetDebugPrefix(), *Equipment->GetName(), *GetNameSafe(Equipment->GetEquipmentDef()));

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(Equipment);
	}

	if (IsActive())
	{
		Equipment->Equip();
	}
}

void UGameEquipmentComponent::OnEquipmentRemoved(UGameEquipment* Equipment)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s Removed equipment: %s"),
		*GetDebugPrefix(), *Equipment->GetName());

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		RemoveReplicatedSubObject(Equipment);
	}

	Equipment->Unequip();
}

void UGameEquipmentComponent::RemoveAllEquipment()
{
	TArray<UGameEquipment*> AllEquipment = GetAllEquipment();
	for (UGameEquipment* Equipment : AllEquipment)
	{
		RemoveEquipment(Equipment);
	}
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

UGameEquipment* UGameEquipmentComponent::FindEquipmentByDef(TSubclassOf<UGameEquipmentDef> EquipmentDef) const
{
	for (const FGameEquipmentListEntry& Entry : EquipmentList.GetEntries())
	{
		UGameEquipment* Equipment = Entry.Equipment;
		if (IsValid(Equipment) && Equipment->GetEquipmentDef() == EquipmentDef)
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

void UGameEquipmentComponent::GetNetExecutionPlan(bool& bOutExecuteServer, bool& bOutExecuteLocal) const
{
	const AActor* Owner = GetOwner();
	if (Owner->HasAuthority())
	{
		// server
		bOutExecuteServer = false;
		bOutExecuteLocal = true;
	}
	else if (Owner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		// autonomous owner
		bOutExecuteServer = true;
		bOutExecuteLocal = false;
	}
	else
	{
		// simulated proxy / not local owner
		bOutExecuteServer = false;
		bOutExecuteLocal = false;
	}
}

void UGameEquipmentComponent::OnPreReplicatedRemove(FGameEquipmentListEntry& Entry)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] %s"), *GetDebugPrefix(), __func__, *Entry.GetDebugString());

	if (Entry.Equipment)
	{
		OnEquipmentRemoved(Entry.Equipment);
	}
}

void UGameEquipmentComponent::OnPostReplicatedAdd(FGameEquipmentListEntry& Entry)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] %s"), *GetDebugPrefix(), __func__, *Entry.GetDebugString());

	if (Entry.Equipment)
	{
		OnEquipmentApplied(Entry.Equipment);
	}
}

void UGameEquipmentComponent::OnPostReplicatedChange(FGameEquipmentListEntry& Entry)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] %s"), *GetDebugPrefix(), __func__, *Entry.GetDebugString());

	if (Entry.Equipment)
	{
		// this should only be called if the entry went from null -> valid, so treat it like OnPostReplicatedAdd
		OnEquipmentApplied(Entry.Equipment);
	}
}

FString UGameEquipmentComponent::GetDebugPrefix() const
{
	return FString::Printf(TEXT("%s[%s]"), *UGameItemStatics::GetNetDebugPrefix(this), *GetReadableName());
}
