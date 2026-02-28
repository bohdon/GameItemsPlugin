// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemAutoSlotRule_Basic.h"

#include "GameItemContainer.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"


namespace GameItems::GameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_AutoSlot_NoReplace, "Item.AutoSlot.NoReplace", "Don't replace existing items when auto-slotting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_AutoSlot_Replace, "Item.AutoSlot.Replace", "Replace existing items when auto-slotting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_AutoSlot_Toggle, "Item.AutoSlot.Toggle", "Remove the item if it already exists in the container when auto-slotting");
}


UGameItemAutoSlotRule_Basic::UGameItemAutoSlotRule_Basic()
	: Priority(1),
	  bReplaceByDefault(true)
{
}

void UGameItemAutoSlotRule_Basic::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemAutoSlotRule_Basic, Priority, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemAutoSlotRule_Basic, bReplaceByDefault, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemAutoSlotRule_Basic, RequireTags, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemAutoSlotRule_Basic, IgnoreTags, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemAutoSlotRule_Basic, Query, Params);
}

bool UGameItemAutoSlotRule_Basic::CanAutoSlot_Implementation(const UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	if (!Item)
	{
		return false;
	}

	const FGameplayTagContainer ItemTags = Item->GetOwnedTags();
	if (!ItemTags.HasAll(RequireTags) || ItemTags.HasAny(IgnoreTags) || !(Query.IsEmpty() || Query.Matches(ItemTags)))
	{
		return false;
	}

	// if NoReplace option is given, return false if we have an existing item we don't want to replace.
	// this is checked here so that auto slot priority can be demoted to 0,
	// making room for a potentially better slot, based on availability
	if (ContextTags.HasTag(GameItems::GameplayTags::Item_AutoSlot_NoReplace))
	{
		const UGameItemContainer* Container = GetContainer();
		check(Container);

		const int32 Slot = GetBestSlotForItem(Item, ContextTags);

		if (const UGameItem* ExistingItem = Container->GetItemAt(Slot))
		{
			if (Item != ExistingItem && !ShouldReplaceItem(Item, ExistingItem, ContextTags))
			{
				return false;
			}
		}
	}

	return Super::CanAutoSlot_Implementation(Item, ContextTags);
}

int32 UGameItemAutoSlotRule_Basic::GetAutoSlotPriorityForItem_Implementation(const UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	if (ContextTags.HasTag(GameItems::GameplayTags::Item_AutoSlot_Toggle))
	{
		// return a super high priority if we have the item equipped and want to toggle
		const UGameItemContainer* Container = GetContainer();
		check(Container);

		if (Container->Contains(Item))
		{
			return INT_MAX;
		}
	}

	return CanAutoSlot(Item, ContextTags) ? Priority : 0;
}

void UGameItemAutoSlotRule_Basic::TryAutoSlotInternal_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	UGameItemContainer* Container = GetContainer();
	check(Container);

	if (Container->Contains(Item))
	{
		if (ContextTags.HasTag(GameItems::GameplayTags::Item_AutoSlot_Toggle))
		{
			// remove the already-slotted item
			Container->RemoveItem(Item);
			return;
		}
	}

	Super::TryAutoSlotInternal_Implementation(Item, ContextTags);
}

int32 UGameItemAutoSlotRule_Basic::GetBestSlotForItem_Implementation(const UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	const int32 NextEmptySlot = GetContainer()->GetNextEmptySlot();
	if (NextEmptySlot != INDEX_NONE)
	{
		// has empty slot, or unlimited slots
		return NextEmptySlot;
	}

	// default to replacing (if possible) the first slot
	return 0;
}

bool UGameItemAutoSlotRule_Basic::ShouldReplaceItem_Implementation(const UGameItem* NewItem, const UGameItem* ExistingItem,
                                                                   const FGameplayTagContainer& ContextTags) const
{
	return NewItem != ExistingItem &&
		(bReplaceByDefault || ContextTags.HasTag(GameItems::GameplayTags::Item_AutoSlot_Replace)) &&
		!ContextTags.HasTag(GameItems::GameplayTags::Item_AutoSlot_NoReplace);
}
