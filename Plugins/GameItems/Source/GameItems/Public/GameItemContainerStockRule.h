// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameItemContainerStockRule.generated.h"

class UGameItem;
class UGameItemContainerComponent;


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
	int32 GetItemMaxCount(const UGameItemContainerComponent* Container, const UGameItem* Item) const;

	/** Return the maximum allowed count for a single stack of an item, or -1 if unlimited. */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetItemStackMaxCount(const UGameItemContainerComponent* Container, const UGameItem* Item) const;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Meta = (InlineEditConditionToggle), Category = "StockRule")
	bool bLimitMaxCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Meta = (EditCondition = "bLimitMaxCount"), Category = "StockRule")
	int32 MaxCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Meta = (InlineEditConditionToggle), Category = "StockRule")
	bool bLimitStackMaxCount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Meta = (EditCondition = "bLimitStackMaxCount"), Category = "StockRule")
	int32 StackMaxCount;

	virtual int32 GetItemMaxCount_Implementation(const UGameItemContainerComponent* Container, const UGameItem* Item) const override;
	virtual int32 GetItemStackMaxCount_Implementation(const UGameItemContainerComponent* Container, const UGameItem* Item) const override;
};
