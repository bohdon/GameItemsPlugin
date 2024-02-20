// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerRule.h"
#include "GameItemTypes.h"
#include "GameItemContainerStockRule_Tags.generated.h"


/**
 * Specifies stock rules by item tags, where rules apply to the first tag that matches an item.
 */
UCLASS(DisplayName = "Stock Rules (Tags)")
class GAMEITEMS_API UGameItemContainerStockRule_Tags : public UGameItemContainerRule
{
	GENERATED_BODY()

public:
	UGameItemContainerStockRule_Tags();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag Stock Rules")
	TMap<FGameplayTag, FGameItemStockRules> StockRules;

	FGameItemStockRules GetStockRulesForItem(const UGameItem* Item) const;

	virtual int32 GetItemMaxCount_Implementation(const UGameItem* Item) const override;
	virtual int32 GetItemStackMaxCount_Implementation(const UGameItem* Item) const override;
};
