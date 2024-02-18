// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemAutoSlotRule_Basic.h"

#include "GameItemContainer.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_AutoSlot_NoReplace, "Item.AutoSlot.NoReplace", "Don't replace existing items when auto-slotting");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_AutoSlot_Replace, "Item.AutoSlot.Replace", "Replace existing items when auto-slotting");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_AutoSlot_Toggle, "Item.AutoSlot.Toggle", "Remove the item if it already exists in the container when auto-slotting");


UGameItemAutoSlotRule_Basic::UGameItemAutoSlotRule_Basic()
	: Priority(1),
	  bReplaceByDefault(true)
{
}

int32 UGameItemAutoSlotRule_Basic::GetAutoSlotPriorityForItem_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	return CanAutoSlot(Item, ContextTags) ? Priority : 0;
}

bool UGameItemAutoSlotRule_Basic::TryAutoSlot_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags, TArray<UGameItem*>& OutItems) const
{
	if (Container->Contains(Item))
	{
		if (ContextTags.HasTag(TAG_Item_AutoSlot_Toggle))
		{
			// remove the already-slotted item
			Container->RemoveItem(Item);
			OutItems.Reset();
			return true;
		}
	}

	return Super::TryAutoSlot_Implementation(Item, ContextTags, OutItems);
}

int32 UGameItemAutoSlotRule_Basic::GetBestSlotForItem_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	const int32 NextEmptySlot = Container->GetNextEmptySlot();
	if (NextEmptySlot != INDEX_NONE)
	{
		// has empty slot, or unlimited slots
		return NextEmptySlot;
	}

	// default to replacing (if possible) the first slot
	return 0;
}

bool UGameItemAutoSlotRule_Basic::ShouldReplaceItem_Implementation(UGameItem* NewItem, UGameItem* ExistingItem, const FGameplayTagContainer& ContextTags) const
{
	return (bReplaceByDefault || ContextTags.HasTag(TAG_Item_AutoSlot_Replace)) && !ContextTags.HasTag(TAG_Item_AutoSlot_NoReplace);
}
