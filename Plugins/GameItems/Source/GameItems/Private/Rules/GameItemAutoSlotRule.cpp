// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemAutoSlotRule.h"

#include "GameItemContainer.h"


int32 UGameItemAutoSlotRule::GetAutoSlotPriorityForItem_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	return 0;
}

bool UGameItemAutoSlotRule::CanAutoSlot_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	return GetContainer()->CanContainItem(Item);
}

void UGameItemAutoSlotRule::TryAutoSlot(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	UGameItemContainer* Container = GetContainer();
	check(Container);

	bool bExecuteServer;
	bool bExecuteLocal;
	Container->GetNetExecutionPlan(bExecuteServer, bExecuteLocal);
	if (bExecuteServer)
	{
		ServerTryAutoSlot(Item, ContextTags);
	}
	if (!bExecuteLocal)
	{
		return;
	}

	TryAutoSlotInternal(Item, ContextTags);
}

void UGameItemAutoSlotRule::ServerTryAutoSlot_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	TryAutoSlot(Item, ContextTags);
}

void UGameItemAutoSlotRule::TryAutoSlotInternal_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	UGameItemContainer* Container = GetContainer();
	check(Container);

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
			return;
		}
	}

	Container->AddItem(Item, Slot);
}

int32 UGameItemAutoSlotRule::GetBestSlotForItem_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const
{
	return INDEX_NONE;
}

bool UGameItemAutoSlotRule::ShouldReplaceItem_Implementation(UGameItem* NewItem, UGameItem* ExistingItem, const FGameplayTagContainer& ContextTags) const
{
	return false;
}
