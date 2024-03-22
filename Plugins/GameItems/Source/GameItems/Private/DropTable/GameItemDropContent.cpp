// Copyright Bohdon Sayre, All Rights Reserved.


#include "DropTable/GameItemDropContent.h"

#include "GameItemDef.h"
#include "GameItemSet.h"
#include "GameItemStatics.h"
#include "DropTable/GameItemDropTableRow.h"
#include "DropTable/GameItemSetEntrySelector.h"


// FGameItemDropContent
// --------------------

bool FGameItemDropContent::ShouldGiveContent() const
{
	return true;
}

void FGameItemDropContent::CheckAndSelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const
{
	if (ShouldGiveContent())
	{
		SelectItems(Context, OutItems);
	}
}

void FGameItemDropContent::SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const
{
}

void FGameItemDropContent::OnDataChanged()
{
}


// FGameItemDropChancedContent
// ---------------------------

bool FGameItemDropChancedContent::ShouldGiveContent() const
{
	const float Value = FMath::FRand();
	return Value <= Chance;
}


// FGameItemDropContent_Combine
// ----------------------------

void FGameItemDropContent_Combine::SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const
{
	for (const TInstancedStruct<FGameItemDropContent>& Entry : Contents)
	{
		if (const FGameItemDropContent* ContentPtr = Entry.GetPtr<FGameItemDropContent>())
		{
			ContentPtr->CheckAndSelectItems(Context, OutItems);
		}
	}
}


// FGameItemDropContent_Select
// ---------------------------

void FGameItemDropContent_Select::SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const
{
	if (WeightedContents.IsEmpty())
	{
		return;
	}

	TArray<float> Probabilities;
	Algo::Transform(WeightedContents, Probabilities, [](const FWeightedGameItemDropContent& Content)
	{
		return Content.Probability;
	});

	const int32 Idx = UGameItemStatics::GetWeightedRandomArrayIndex(Probabilities);
	check(Idx != INDEX_NONE);

	const FWeightedGameItemDropContent& WeightedContent = WeightedContents[Idx];
	if (const FGameItemDropContent* ContentPtr = WeightedContent.Content.GetPtr<FGameItemDropContent>())
	{
		ContentPtr->CheckAndSelectItems(Context, OutItems);
	}
}


// FGameItemDropContent_Item
// -------------------------

void FGameItemDropContent_Item::SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const
{
	if (TSubclassOf<UGameItemDef> ItemDefClass = ItemDef.LoadSynchronous())
	{
		OutItems.Emplace(ItemDefClass, Count);
	}
}


// FGameItemDropContent_ItemSet
// ----------------------------

void FGameItemDropContent_ItemSet::SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const
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

	FGameItemDropContext SubContext = Context;
	if (Params.IsValid())
	{
		// override parent params
		SubContext.Params = Params;
	}

	const UGameItemSetEntrySelector* SelectorCDO = GetDefault<UGameItemSetEntrySelector>(SelectorClass);
	SelectorCDO->SelectItems(SubContext, ItemSetPtr, OutItems);
}

// FGameItemDropContent_DropTableEntry
// -----------------------------------

void FGameItemDropContent_DropTableEntry::SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const
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

	FGameItemDropContext SubContext = Context;
	if (Params.IsValid())
	{
		// override parent params
		SubContext.Params = Params;
	}

	UGameItemStatics::SelectItemsFromDropTableRow(SubContext, *Row, OutItems);
}
