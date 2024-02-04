// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerInterface.h"

#include "GameItemContainer.h"
#include "GameItemSettings.h"


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
	const TArray<UGameItemContainer*> AllItemContainers = GetAllItemContainers();
	for (UGameItemContainer* ItemContainer : AllItemContainers)
	{
		if (ItemContainer->ContainerId == ContainerId)
		{
			return ItemContainer;
		}
	}

	return nullptr;
}

UGameItemContainer* IGameItemContainerInterface::GetDefaultItemContainer() const
{
	return GetItemContainer(UGameItemSettings::GetDefaultContainerId());
}
