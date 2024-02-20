// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerStockRule_Tags.h"
#include "GameItem.h"
#include "GameItemDef.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerStockRule_Tags)


UGameItemContainerStockRule_Tags::UGameItemContainerStockRule_Tags()
{
}

FGameItemStockRules UGameItemContainerStockRule_Tags::GetStockRulesForItem(const UGameItem* Item) const
{
	const UGameItemDef* ItemDefCDO = Item ? Item->GetItemDefCDO() : nullptr;
	if (!ItemDefCDO)
	{
		return FGameItemStockRules();
	}

	for (FGameplayTag ItemTag : ItemDefCDO->OwnedTags)
	{
		if (StockRules.Contains(ItemTag))
		{
			return StockRules[ItemTag];
		}
	}
	return FGameItemStockRules();
}

int32 UGameItemContainerStockRule_Tags::GetItemMaxCount_Implementation(const UGameItem* Item) const
{
	const FGameItemStockRules ItemStockRules = GetStockRulesForItem(Item);
	return ItemStockRules.bLimitCount ? ItemStockRules.MaxCount : -1;
}

int32 UGameItemContainerStockRule_Tags::GetItemStackMaxCount_Implementation(const UGameItem* Item) const
{
	const FGameItemStockRules ItemStockRules = GetStockRulesForItem(Item);
	return ItemStockRules.bLimitStackCount ? ItemStockRules.StackMaxCount : -1;
}
