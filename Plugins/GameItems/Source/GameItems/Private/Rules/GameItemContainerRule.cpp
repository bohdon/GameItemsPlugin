// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerRule.h"

#include "GameItem.h"
#include "GameItemContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerRule)


UGameItemContainerRule::UGameItemContainerRule()
{
}

void UGameItemContainerRule::Initialize()
{
	Container = GetTypedOuter<UGameItemContainer>();
	check(Container);
}

void UGameItemContainerRule::Uninitialize()
{
}

bool UGameItemContainerRule::IsChild_Implementation() const
{
	return false;
}

UWorld* UGameItemContainerRule::GetWorld() const
{
	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

bool UGameItemContainerRule::CanContainItem_Implementation(const UGameItem* Item) const
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
