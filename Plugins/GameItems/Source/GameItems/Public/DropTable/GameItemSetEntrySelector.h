// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemDropContext.h"
#include "GameItemTypes.h"
#include "UObject/Object.h"
#include "GameItemSetEntrySelector.generated.h"

class UGameItemSet;


/**
 * Handles the filtering and selection of entries from an item set.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const)
class GAMEITEMS_API UGameItemSetEntrySelector : public UObject
{
	GENERATED_BODY()

public:
	/** Use the DropRules item fragments when available to calculate item probabilities and conditions. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bUseDropRules = true;

	/** Select one or more items from a game item set. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SelectItems(const FGameItemDropContext& Context, const UGameItemSet* ItemSet, TArray<FGameItemDefStack>& OutItems) const;

	/** Return true if an item passes all conditions and can be selected. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanSelectItem(const FGameItemDropContext& Context, const UGameItemSet* ItemSet, const FGameItemDefStack& Entry) const;

	/** Return the relative probability to use for selecting an item in the set. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetItemProbability(const FGameItemDropContext& Context, const UGameItemSet* ItemSet, const FGameItemDefStack& Entry) const;

protected:
	virtual void GetFilteredAndWeightedItems(const FGameItemDropContext& Context, const UGameItemSet* ItemSet,
	                                         TArray<FGameItemDefStack>& OutFilteredItems, TArray<float>& OutProbabilities) const;
};


/**
 * Selects all items from the set.
 */
UCLASS(BlueprintType, DisplayName = "All")
class UGameItemSetEntrySelector_All : public UGameItemSetEntrySelector
{
	GENERATED_BODY()

public:
	virtual void SelectItems_Implementation(const FGameItemDropContext& Context, const UGameItemSet* ItemSet,
	                                        TArray<FGameItemDefStack>& OutItems) const override;
};


/**
 * Selects a random item from the set.
 */
UCLASS(BlueprintType, DisplayName = "Random")
class UGameItemSetEntrySelector_Random : public UGameItemSetEntrySelector
{
	GENERATED_BODY()

public:
	/** Use the EconValue item fragments when available, as well as the drop context's target value to calculate item quantities. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bUseEconValue = true;

	virtual void SelectItems_Implementation(const FGameItemDropContext& Context, const UGameItemSet* ItemSet,
	                                        TArray<FGameItemDefStack>& OutItems) const override;
};
