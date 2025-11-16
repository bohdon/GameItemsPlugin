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
		return Container->GetContainerId();
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

UGameItemContainer* IGameItemContainerInterface::GetDefaultContainerForItem(TSubclassOf<UGameItemDef> ItemDef) const
{
	// check if the default is compatible
	UGameItemContainer* DefaultContainer = GetDefaultItemContainer();
	if (DefaultContainer && DefaultContainer->CanContainItemByDef(ItemDef))
	{
		return DefaultContainer;
	}

	// otherwise return the first parent container that supports the item
	for (UGameItemContainer* Container : GetAllItemContainers())
	{
		if (Container != DefaultContainer && !Container->IsChild() && Container->CanContainItemByDef(ItemDef))
		{
			return Container;
		}
	}

	return nullptr;
}
