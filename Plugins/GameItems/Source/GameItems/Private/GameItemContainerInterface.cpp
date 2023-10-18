// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerInterface.h"

#include "GameItemContainerComponent.h"


UGameItemContainerComponent* IGameItemContainerInterface::GetItemContainerComponent(FGameplayTag IdTag) const
{
	const TArray<UGameItemContainerComponent*> AllItemContainers = GetAllItemContainerComponent();
	for (UGameItemContainerComponent* ItemContainer : AllItemContainers)
	{
		if (ItemContainer->IdTag == IdTag)
		{
			return ItemContainer;
		}
	}

	return nullptr;
}
