// Copyright Bohdon Sayre, All Rights Reserved.


#include "DropTable/GameItemSetEntrySelector.h"

#include "GameItemDef.h"
#include "GameItemSet.h"
#include "GameItemsModule.h"
#include "GameItemStatics.h"
#include "Fragments/GameItemFragment_DropRules.h"
#include "Fragments/GameItemFragment_EconValue.h"


// UGameItemSetEntrySelector
// -------------------------

void UGameItemSetEntrySelector::SelectItems_Implementation(const FGameItemDropContext& Context, const UGameItemSet* ItemSet,
                                                           TArray<FGameItemDefStack>& OutItems) const
{
}

bool UGameItemSetEntrySelector::CanSelectItem_Implementation(const FGameItemDropContext& Context, const UGameItemSet* ItemSet,
                                                             const FGameItemDefStack& Entry) const
{
	if (!Entry.ItemDef)
	{
		return false;
	}

	if (bUseDropRules)
	{
		const UGameItemDef* ItemDefCDO = GetDefault<UGameItemDef>(Entry.ItemDef);
		if (const UGameItemFragment_DropRules* DropRulesFrag = ItemDefCDO->FindFragment<UGameItemFragment_DropRules>())
		{
			if (!DropRulesFrag->IsConditionMet(Context))
			{
				return false;
			}
		}
	}

	return true;
}

float UGameItemSetEntrySelector::GetItemProbability_Implementation(const FGameItemDropContext& Context,
                                                                   const UGameItemSet* ItemSet, const FGameItemDefStack& Entry) const
{
	check(Entry.ItemDef);

	if (bUseDropRules)
	{
		const UGameItemDef* ItemDefCDO = GetDefault<UGameItemDef>(Entry.ItemDef);
		if (const UGameItemFragment_DropRules* DropRulesFrag = ItemDefCDO->FindFragment<UGameItemFragment_DropRules>())
		{
			return DropRulesFrag->GetProbability(Context);
		}
	}

	return 1.f;
}

void UGameItemSetEntrySelector::GetFilteredAndWeightedItems(const FGameItemDropContext& Context, const UGameItemSet* ItemSet,
                                                            TArray<FGameItemDefStack>& OutFilteredItems, TArray<float>& OutProbabilities) const
{
	OutFilteredItems.Reset();
	OutFilteredItems = ItemSet->Items.FilterByPredicate([&](const FGameItemDefStack& Entry)
	{
		return Entry.ItemDef && CanSelectItem(Context, ItemSet, Entry);
	});

	OutProbabilities.Reset(OutFilteredItems.Num());
	Algo::Transform(OutFilteredItems, OutProbabilities, [&](const FGameItemDefStack& Entry)
	{
		return GetItemProbability(Context, ItemSet, Entry);
	});
}


// UGameItemSetEntrySelector_All
// -----------------------------

void UGameItemSetEntrySelector_All::SelectItems_Implementation(const FGameItemDropContext& Context, const UGameItemSet* ItemSet,
                                                               TArray<FGameItemDefStack>& OutItems) const
{
	check(ItemSet);
	OutItems.Append(ItemSet->Items);
}


// UGameItemSetEntrySelector_Random
// --------------------------------

void UGameItemSetEntrySelector_Random::SelectItems_Implementation(const FGameItemDropContext& Context, const UGameItemSet* ItemSet,
                                                                  TArray<FGameItemDefStack>& OutItems) const
{
	check(ItemSet);
	TArray<FGameItemDefStack> FilteredItems;
	TArray<float> Probabilities;
	GetFilteredAndWeightedItems(Context, ItemSet, FilteredItems, Probabilities);

	if (FilteredItems.IsEmpty())
	{
		return;
	}

	const int32 RandIdx = UGameItemStatics::GetWeightedRandomArrayIndex(Probabilities);
	check(RandIdx != INDEX_NONE);

	FGameItemDefStack Item = FilteredItems[RandIdx];
	if (!Item.ItemDef)
	{
		UE_LOG(LogGameItems, Error, TEXT("Selected entry with null ItemDef from item set: %s[%d]"), *ItemSet->GetName(), RandIdx);
		return;
	}

	if (bUseEconValue)
	{
		const UGameItemDef* ItemDefCDO = GetDefault<UGameItemDef>(Item.ItemDef);
		if (const UGameItemFragment_EconValue* EconValueFrag = ItemDefCDO->FindFragment<UGameItemFragment_EconValue>())
		{
			if (const FGameItemDropParams_EconValue* EconParams = Context.Params.GetPtr<FGameItemDropParams_EconValue>())
			{
				const float Value = FMath::Max(FMath::FRandRange(EconParams->EconValueMin, EconParams->EconValueMax), 0.f);
				Item.Count = EconValueFrag->GetCountForValue(Value);
			}
		}
	}

	OutItems.Add(Item);
}
