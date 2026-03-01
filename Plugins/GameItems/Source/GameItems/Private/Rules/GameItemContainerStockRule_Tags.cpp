// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerStockRule_Tags.h"
#include "GameItem.h"
#include "GameItemDef.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerStockRule_Tags)


UGameItemContainerStockRule_Tags::UGameItemContainerStockRule_Tags()
{
}

int32 UGameItemContainerStockRule_Tags::GetItemMaxCount_Implementation(const UGameItem* Item) const
{
	return FindLimitForItem(Item, ContainerLimits).GetMaxCount(-1);
}

int32 UGameItemContainerStockRule_Tags::GetItemStackMaxCount_Implementation(const UGameItem* Item) const
{
	return FindLimitForItem(Item, StackLimits).GetMaxCount(-1);
}

FGameItemCountLimit UGameItemContainerStockRule_Tags::FindLimitForItem(const UGameItem* Item, const TMap<FGameplayTag, FGameItemCountLimit>& Limits)
{
	const UGameItemDef* ItemDefCDO = Item ? Item->GetItemDefCDO() : nullptr;
	if (!ItemDefCDO)
	{
		return FGameItemCountLimit();
	}

	for (const TTuple<FGameplayTag, FGameItemCountLimit>& Entry : Limits)
	{
		if (ItemDefCDO->OwnedTags.HasTag(Entry.Key))
		{
			return Entry.Value;
		}
	}
	return FGameItemCountLimit();
}
