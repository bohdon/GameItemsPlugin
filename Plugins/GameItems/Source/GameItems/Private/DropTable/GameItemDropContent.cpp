// Copyright Bohdon Sayre, All Rights Reserved.


#include "DropTable/GameItemDropContent.h"

#include "GameItemDef.h"
#include "GameItemSet.h"
#include "GameItemStatics.h"
#include "DropTable/GameItemDropTableRow.h"
#include "DropTable/GameItemSetEntrySelector.h"


// FGameItemDropProbability
// ------------------------

void FGameItemDropProbability::UpdateDerivedValue()
{
	if (bUseOdds)
	{
		Probability = 1.f / Odds;
	}
	else
	{
		Odds = 1.f / Probability;
	}
}

bool FGameItemDropProbability::RandomCheck() const
{
	const float Threshold = bUseOdds ? 1.f / Odds : Probability;
	const float Value = FMath::FRand();
	return Value <= Threshold;
}


// FGameItemDropContent
// --------------------

void FGameItemDropContent::CheckAndSelectItems(TArray<FGameItemDefStack>& OutItems) const
{
	if (Probability.RandomCheck())
	{
		SelectItems(OutItems);
	}
}

void FGameItemDropContent::SelectItems(TArray<FGameItemDefStack>& OutItems) const
{
}

void FGameItemDropContent::OnDataChanged()
{
	Probability.UpdateDerivedValue();
}


// FGameItemDropContent_Item
// -------------------------

void FGameItemDropContent_Item::SelectItems(TArray<FGameItemDefStack>& OutItems) const
{
	if (TSubclassOf<UGameItemDef> ItemDefClass = ItemDef.LoadSynchronous())
	{
		OutItems.Emplace(ItemDefClass, Count);
	}
}


// FGameItemDropContent_ItemSet
// ----------------------------

void FGameItemDropContent_ItemSet::SelectItems(TArray<FGameItemDefStack>& OutItems) const
{
	if (!ItemSet.IsNull())
	{
		OutItems.Append(ItemSet.LoadSynchronous()->Items);
	}
}


// FGameItemDropContent_ItemSetEntry
// ---------------------------------

void FGameItemDropContent_ItemSetEntry::SelectItems(TArray<FGameItemDefStack>& OutItems) const
{
	const UGameItemSet* ItemSetPtr = ItemSet.LoadSynchronous();
	if (!ItemSetPtr || ItemSetPtr->Items.IsEmpty())
	{
		return;
	}

	if (!SelectorClass)
	{
		return;
	}

	const UGameItemSetEntrySelector* SelectorCDO = GetDefault<UGameItemSetEntrySelector>(SelectorClass);
	const FGameItemDefStack SelectedItem = SelectorCDO->SelectEntry(ItemSetPtr);
	OutItems.Add(SelectedItem);
}

// FGameItemDropContent_DropTableEntry
// -----------------------------------

void FGameItemDropContent_DropTableEntry::SelectItems(TArray<FGameItemDefStack>& OutItems) const
{
	static FString ContextString(TEXT("FGameItemDropContent_DropTableEntry::SelectItems"));
	if (DropTableRow.IsNull())
	{
		return;
	}

	const FGameItemDropTableRow* Row = DropTableRow.GetRow<FGameItemDropTableRow>(ContextString);
	if (!Row)
	{
		return;
	}

	UGameItemStatics::SelectItemsFromDropTableRow(*Row, OutItems);
}
