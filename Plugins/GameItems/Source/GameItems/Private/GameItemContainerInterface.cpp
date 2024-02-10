// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerInterface.h"

#include "GameItemContainer.h"
#include "GameItemSettings.h"
#include "GameItemStatics.h"


TArray<FGameplayTag> IGameItemContainerInterface::GetAllItemContainerIds() const
{
	const TArray<UGameItemContainer*> AllContainers = GetAllItemContainers();

	TArray<FGameplayTag> Result;
	Algo::Transform(AllContainers, Result, [](const UGameItemContainer* Container)
	{
		return Container->ContainerId;
	});
	return Result;
}

UGameItemContainer* IGameItemContainerInterface::GetItemContainer(FGameplayTag ContainerId) const
{
	return UGameItemStatics::GetItemContainerById(GetAllItemContainers(), ContainerId);
}

UGameItemContainer* IGameItemContainerInterface::GetDefaultItemContainer() const
{
	return GetItemContainer(UGameItemSettings::GetDefaultContainerId());
}
