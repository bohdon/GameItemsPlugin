// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "UObject/Object.h"
#include "GameItemSetEntrySelector.generated.h"

class UGameItemSet;


/**
 * Handles the filtering and selection of an entry from an item set.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const)
class GAMEITEMS_API UGameItemSetEntrySelector : public UObject
{
	GENERATED_BODY()

public:
	/** Select an entry from a game item set. */
	virtual FGameItemDefStack SelectEntry(const UGameItemSet* ItemSet) const;

	/** Return true if an entry passes all conditions and can be selected. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanSelectEntry(const UGameItemSet* ItemSet, const FGameItemDefStack& Entry) const;

	/** Return the relative probability to use for selecting an entry. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetEntryProbability(const UGameItemSet* ItemSet, const FGameItemDefStack& Entry) const;

protected:
	virtual void GetFilteredAndWeightedItems(const UGameItemSet* ItemSet, TArray<FGameItemDefStack>& OutFilteredItems, TArray<float>& OutProbabilities) const;
};


/**
 * Selects a random item from the set.
 */
UCLASS(BlueprintType, DisplayName = "Random")
class UGameItemSetEntrySelector_Random : public UGameItemSetEntrySelector
{
	GENERATED_BODY()

public:
	virtual FGameItemDefStack SelectEntry(const UGameItemSet* ItemSet) const override;
};
