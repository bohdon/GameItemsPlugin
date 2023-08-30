// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerStockRule.h"


// UGameItemContainerStockRule
// ---------------------------

UGameItemContainerStockRule::UGameItemContainerStockRule()
{
}

int32 UGameItemContainerStockRule::GetItemMaxCount_Implementation(const UGameItemContainerComponent* Container, const UGameItem* Item) const
{
	return -1;
}

int32 UGameItemContainerStockRule::GetItemStackMaxCount_Implementation(const UGameItemContainerComponent* Container, const UGameItem* Item) const
{
	return -1;
}


// UGameItemContainerStockRule_Simple
// ----------------------------------

UGameItemContainerStockRule_Simple::UGameItemContainerStockRule_Simple()
	: bLimitMaxCount(false),
	  MaxCount(100),
	  bLimitStackMaxCount(false),
	  StackMaxCount(100)
{
}

int32 UGameItemContainerStockRule_Simple::GetItemMaxCount_Implementation(const UGameItemContainerComponent* Container, const UGameItem* Item) const
{
	return bLimitMaxCount ? MaxCount : -1;
}

int32 UGameItemContainerStockRule_Simple::GetItemStackMaxCount_Implementation(const UGameItemContainerComponent* Container, const UGameItem* Item) const
{
	return bLimitStackMaxCount ? StackMaxCount : -1;
}
