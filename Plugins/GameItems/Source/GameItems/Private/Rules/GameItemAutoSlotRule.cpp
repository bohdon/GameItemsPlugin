// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemAutoSlotRule.h"

#include "GameItemContainer.h"


int32 UGameItemAutoSlotRule::GetAutoSlotPriorityForItem_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	return 0;
}

bool UGameItemAutoSlotRule::CanAutoSlot_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	return Container->CanContainItem(Item);
}

bool UGameItemAutoSlotRule::TryAutoSlot_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags, TArray<UGameItem*>& OutItems) const
{
	OutItems.Reset();

	const int32 Slot = GetBestSlotForItem(Item, ContextTags);

	// check for (and possibly remove) existing item
	if (UGameItem* ExistingItem = Container->GetItemAt(Slot))
	{
		if (ShouldReplaceItem(Item, ExistingItem, ContextTags))
		{
			// remove the existing item
			Container->RemoveItemAt(Slot);
		}
		else
		{
			// don't replace
			return false;
		}
	}

	OutItems = Container->AddItem(Item, Slot);
	return !OutItems.IsEmpty();
}

int32 UGameItemAutoSlotRule::GetBestSlotForItem_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	return INDEX_NONE;
}

bool UGameItemAutoSlotRule::ShouldReplaceItem_Implementation(UGameItem* NewItem, UGameItem* ExistingItem, const FGameplayTagContainer& ContextTags) const
{
	return false;
}
