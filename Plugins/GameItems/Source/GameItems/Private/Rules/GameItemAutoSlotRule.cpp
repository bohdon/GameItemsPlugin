// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemAutoSlotRule.h"

#include "GameItemStatics.h"


// UGameItemAutoSlotRule
// ---------------------

TArray<UGameItem*> UGameItemAutoSlotRule::TryAutoSlotItem_Implementation(UGameItem* Item, const TArray<UGameItemContainer*>& Containers,
                                                                         const FGameplayTagContainer& ContextTags) const
{
	return TArray<UGameItem*>();
}

TArray<UGameItem*> UGameItemAutoSlotRule::AutoSlotItem_Implementation(UGameItem* Item, UGameItemContainer* Container, int32 Slot, bool bShouldReplace) const
{
	if (!Item || !Container || !Container->IsValidSlot(Slot))
	{
		return TArray<UGameItem*>();
	}

	// check for replacement
	if (!Container->IsSlotEmpty(Slot))
	{
		if (bShouldReplace)
		{
			// remove the item
			Container->RemoveItemAt(Slot);
		}
		else
		{
			return TArray<UGameItem*>();
		}
	}

	return Container->AddItem(Item, Slot);
}


// UGameItemAutoSlotRule_ItemTags
// ------------------------------

TArray<UGameItem*> UGameItemAutoSlotRule_ItemTags::TryAutoSlotItem_Implementation(UGameItem* Item, const TArray<UGameItemContainer*>& Containers,
                                                                                  const FGameplayTagContainer& ContextTags) const
{
	if (!Item || !Item->GetOwnedTags().HasAny(ItemTags))
	{
		return TArray<UGameItem*>();
	}

	// find target container
	UGameItemContainer* Container = UGameItemStatics::GetItemContainerById(Containers, ContainerId);
	if (!Container)
	{
		return TArray<UGameItem*>();
	}

	// determine target slot
	int32 Slot = Container->GetNextEmptySlot();
	if (Slot == INDEX_NONE)
	{
		Slot = 0;
	}

	const bool bShouldReplace = ContextTags.HasAny(ReplaceContextTags);
	return AutoSlotItem(Item, Container, Slot, bShouldReplace);
}
