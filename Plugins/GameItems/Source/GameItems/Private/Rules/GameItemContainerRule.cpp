// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerRule.h"

#include "GameItem.h"
#include "GameItemContainer.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerRule)


UGameItemContainerRule::UGameItemContainerRule()
{
}

int32 UGameItemContainerRule::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UGameItemContainerRule::CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter() != nullptr);

	AActor* Actor = GetTypedOuter<AActor>();
	UNetDriver* NetDriver = Actor ? Actor->GetNetDriver() : nullptr;
	if (!NetDriver)
	{
		return false;
	}
	NetDriver->ProcessRemoteFunction(Actor, Function, Parms, OutParms, Stack, this);
	return true;
}

ENetRole UGameItemContainerRule::GetNetRole() const
{
	if (const AActor* Actor = GetTypedOuter<AActor>())
	{
		return Actor->GetLocalRole();
	}
	return ENetRole::ROLE_None;
}

UGameItemContainer* UGameItemContainerRule::GetContainer() const
{
	return GetTypedOuter<UGameItemContainer>();
}

bool UGameItemContainerRule::IsChild_Implementation() const
{
	return false;
}

UWorld* UGameItemContainerRule::GetWorld() const
{
	if (const UObject* Outer = GetOuter())
	{
		return Outer->GetWorld();
	}
	return nullptr;
}

bool UGameItemContainerRule::CanContainItem_Implementation(const UGameItem* Item) const
{
	return true;
}

bool UGameItemContainerRule::CanContainItemByDef_Implementation(TSubclassOf<UGameItemDef> ItemDef) const
{
	return true;
}

int32 UGameItemContainerRule::GetItemMaxCount_Implementation(const UGameItem* Item) const
{
	return -1;
}

int32 UGameItemContainerRule::GetItemStackMaxCount_Implementation(const UGameItem* Item) const
{
	return -1;
}
