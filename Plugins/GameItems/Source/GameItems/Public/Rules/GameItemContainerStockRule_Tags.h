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

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#if WITH_EDITOR
	virtual void PostLoad() override;
#endif

	/** Container max count limits to apply (in order of priority) to any item matching the tags. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Tag Stock Rules", DisplayName = "Container Limits")
	TArray<FGameItemTagCountLimit> ContainerLimitsArray;

	/** Stack max count limits to apply (in order of priority) to any item matching the tags. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Tag Stock Rules", DisplayName = "Stock Limits")
	TArray<FGameItemTagCountLimit> StackLimitsArray;

	virtual int32 GetItemMaxCount_Implementation(const UGameItem* Item) const override;
	virtual int32 GetItemStackMaxCount_Implementation(const UGameItem* Item) const override;

protected:
	static FGameItemCountLimit FindLimitForItem(const UGameItem* Item, const TArray<FGameItemTagCountLimit>& Limits);

private:
	UPROPERTY()
	TMap<FGameplayTag, FGameItemCountLimit> ContainerLimits_DEPRECATED;

	UPROPERTY()
	TMap<FGameplayTag, FGameItemCountLimit> StackLimits_DEPRECATED;
};
