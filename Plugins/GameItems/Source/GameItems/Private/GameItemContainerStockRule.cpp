// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerStockRule.h"

#include "GameItem.h"
#include "GameItemDef.h"


// UGameItemContainerStockRule
// ---------------------------

UGameItemContainerStockRule::UGameItemContainerStockRule()
{
}

int32 UGameItemContainerStockRule::GetItemMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const
{
	return -1;
}

int32 UGameItemContainerStockRule::GetItemStackMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const
{
	return -1;
}


// UGameItemContainerStockRule_Simple
// ----------------------------------

UGameItemContainerStockRule_Simple::UGameItemContainerStockRule_Simple()
{
}

int32 UGameItemContainerStockRule_Simple::GetItemMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const
{
	return StockRules.bLimitCount ? StockRules.MaxCount : -1;
}

int32 UGameItemContainerStockRule_Simple::GetItemStackMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const
{
	return StockRules.bLimitStackCount ? StockRules.StackMaxCount : -1;
}


// UGameItemContainerStockRule_Tags
// --------------------------------

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

int32 UGameItemContainerStockRule_Tags::GetItemMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const
{
	const FGameItemStockRules ItemStockRules = GetStockRulesForItem(Item);
	return ItemStockRules.bLimitCount ? ItemStockRules.MaxCount : -1;
}

int32 UGameItemContainerStockRule_Tags::GetItemStackMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const
{
	const FGameItemStockRules ItemStockRules = GetStockRulesForItem(Item);
	return ItemStockRules.bLimitStackCount ? ItemStockRules.StackMaxCount : -1;
}
