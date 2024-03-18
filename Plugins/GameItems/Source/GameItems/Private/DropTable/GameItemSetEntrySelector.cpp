// Copyright Bohdon Sayre, All Rights Reserved.


#include "DropTable/GameItemSetEntrySelector.h"

#include "GameItemSet.h"
#include "GameItemStatics.h"


// UGameItemSetEntrySelector
// -------------------------

FGameItemDefStack UGameItemSetEntrySelector::SelectEntry(const UGameItemSet* ItemSet) const
{
	return FGameItemDefStack();
}

bool UGameItemSetEntrySelector::CanSelectEntry_Implementation(const UGameItemSet* ItemSet, const FGameItemDefStack& Entry) const
{
	return true;
}

float UGameItemSetEntrySelector::GetEntryProbability_Implementation(const UGameItemSet* ItemSet, const FGameItemDefStack& Entry) const
{
	return 1.f;
}

void UGameItemSetEntrySelector::GetFilteredAndWeightedItems(const UGameItemSet* ItemSet,
                                                            TArray<FGameItemDefStack>& OutFilteredItems,
                                                            TArray<float>& OutProbabilities) const
{
	OutFilteredItems.Reset();
	OutFilteredItems = ItemSet->Items.FilterByPredicate([&](const FGameItemDefStack& Entry)
	{
		return CanSelectEntry(ItemSet, Entry);
	});

	OutProbabilities.Reset(OutFilteredItems.Num());
	Algo::Transform(OutFilteredItems, OutProbabilities, [&](const FGameItemDefStack& Entry)
	{
		return GetEntryProbability(ItemSet, Entry);
	});
}


// UGameItemSetEntrySelector_Random
// --------------------------------
FGameItemDefStack UGameItemSetEntrySelector_Random::SelectEntry(const UGameItemSet* ItemSet) const
{
	TArray<FGameItemDefStack> FilteredItems;
	TArray<float> Probabilities;
	GetFilteredAndWeightedItems(ItemSet, FilteredItems, Probabilities);

	if (FilteredItems.IsEmpty())
	{
		return FGameItemDefStack();
	}

	const int32 RandIdx = UGameItemStatics::GetWeightedRandomArrayIndex(Probabilities);
	check(RandIdx != INDEX_NONE);

	return FilteredItems[RandIdx];
}
