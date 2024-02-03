// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSet.h"

#include "GameItemContainer.h"


void UGameItemSet::AddToContainer(UGameItemContainer* Container) const
{
	if (!Container)
	{
		return;
	}

	for (const FGameItemDefStack& DefaultItem : Items)
	{
		Container->AddNewItem(DefaultItem.ItemDef, DefaultItem.Count);
	}
}
