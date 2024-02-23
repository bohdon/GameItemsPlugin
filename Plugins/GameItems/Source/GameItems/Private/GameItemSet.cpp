// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSet.h"

#include "GameItemContainer.h"
#include "GameItemSubsystem.h"


void UGameItemSet::AddToContainer(UGameItemContainer* Container) const
{
	if (!Container)
	{
		return;
	}

	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(Container);
	if (!ItemSubsystem)
	{
		return;
	}

	for (const FGameItemDefStack& Item : Items)
	{
		ItemSubsystem->CreateItemInContainer(Container, Item.ItemDef, Item.Count);
	}
}
