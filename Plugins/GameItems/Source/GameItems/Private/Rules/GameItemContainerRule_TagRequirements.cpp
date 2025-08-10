﻿// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerRule_TagRequirements.h"

#include "GameItem.h"
#include "GameItemDef.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerRule_TagRequirements)


bool UGameItemContainerRule_TagRequirements::CanContainItem_Implementation(const UGameItem* Item) const
{
	return CanContainItemByDef_Implementation(Item->GetItemDef());
}

bool UGameItemContainerRule_TagRequirements::CanContainItemByDef_Implementation(TSubclassOf<UGameItemDef> ItemDef) const
{
	const FGameplayTagContainer& ItemTags = GetDefault<UGameItemDef>(ItemDef)->OwnedTags;
	return ItemTags.HasAll(RequireTags) && !ItemTags.HasAny(IgnoreTags) && (Query.IsEmpty() || Query.Matches(ItemTags));
}
