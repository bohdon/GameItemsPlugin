// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemAutoSlotRule_Basic.h"

#include "GameItemContainer.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_AutoSlot_NoReplace, "Item.AutoSlot.NoReplace", "Don't replace existing items when auto-slotting");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_AutoSlot_Replace, "Item.AutoSlot.Replace", "Replace existing items when auto-slotting");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Item_AutoSlot_Toggle, "Item.AutoSlot.Toggle", "Remove the item if it already exists in the container when auto-slotting");


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

bool UGameItemAutoSlotRule_Basic::CanAutoSlot_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	const FGameplayTagContainer ItemTags = Item->GetOwnedTags();
	if (!ItemTags.HasAll(RequireTags) || ItemTags.HasAny(IgnoreTags) || !(Query.IsEmpty() || Query.Matches(ItemTags)))
	{
		return false;
	}

	return Super::CanAutoSlot_Implementation(Item, ContextTags);
}

int32 UGameItemAutoSlotRule_Basic::GetAutoSlotPriorityForItem_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	return CanAutoSlot(Item, ContextTags) ? Priority : 0;
}

bool UGameItemAutoSlotRule_Basic::TryAutoSlot_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags, TArray<UGameItem*>& OutItems) const
{
	UGameItemContainer* Container = GetContainer();
	check(Container);
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
	const int32 NextEmptySlot = GetContainer()->GetNextEmptySlot();
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