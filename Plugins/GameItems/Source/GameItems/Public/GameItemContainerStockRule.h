// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "UObject/Object.h"
#include "GameItemContainerStockRule.generated.h"

class UGameItem;
class UGameItemContainer;


/**
 * Defines rules that restrict what and how many items can exist in a container.
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, Abstract)
class GAMEITEMS_API UGameItemContainerStockRule : public UObject
{
	GENERATED_BODY()

public:
	UGameItemContainerStockRule();

	/** Return the maximum allowed count for an item, or -1 if unlimited. */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetItemMaxCount(const UGameItemContainer* Container, const UGameItem* Item) const;

	/** Return the maximum allowed count for a single stack of an item, or -1 if unlimited. */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetItemStackMaxCount(const UGameItemContainer* Container, const UGameItem* Item) const;
};


/**
 * Specifies a flat limit for max count and stack max count of any items.
 */
UCLASS()
class GAMEITEMS_API UGameItemContainerStockRule_Simple : public UGameItemContainerStockRule
{
	GENERATED_BODY()

public:
	UGameItemContainerStockRule_Simple();

	/** The stock rules for all items. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (ShowOnlyInnerProperties), Category = "GameItem")
	FGameItemStockRules StockRules;

	virtual int32 GetItemMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const override;
	virtual int32 GetItemStackMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const override;
};


/**
 * Specifies stock rules by item tags, where rules apply to the first tag that matches an item.
 */
UCLASS()
class GAMEITEMS_API UGameItemContainerStockRule_Tags : public UGameItemContainerStockRule
{
	GENERATED_BODY()

public:
	UGameItemContainerStockRule_Tags();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameItem")
	TMap<FGameplayTag, FGameItemStockRules> StockRules;

	FGameItemStockRules GetStockRulesForItem(const UGameItem* Item) const;

	virtual int32 GetItemMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const override;
	virtual int32 GetItemStackMaxCount_Implementation(const UGameItemContainer* Container, const UGameItem* Item) const override;
};
