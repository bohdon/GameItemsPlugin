// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSet.h"

#include "GameItemContainerComponent.h"


void UGameItemSet::AddToContainer(UGameItemContainerComponent* ItemContainer) const
{
	if (!ItemContainer)
	{
		return;
	}

	for (const FGameItemDefStack& DefaultItem : Items)
	{
		ItemContainer->AddNewItem(DefaultItem.ItemDef, DefaultItem.Count);
	}
}
