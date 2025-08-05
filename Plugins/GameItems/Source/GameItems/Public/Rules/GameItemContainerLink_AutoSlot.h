﻿// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "GameItemContainerLink.h"
#include "GameItemContainerLink_AutoSlot.generated.h"


/**
 * Tries to auto-slot an item in this container when it's added to the linked container.
 * By default, this rule includes the 'Item.AutoSlot.NoReplace' context tag when auto-slotting.
 */
UCLASS(DisplayName = "Link (Trigger Auto Slot)")
class UGameItemContainerLink_AutoSlot : public UGameItemContainerLink
{
	GENERATED_BODY()

public:
	UGameItemContainerLink_AutoSlot();

	/** Items must match this query to be auto-slotted. If empty, all compatible items are auto-slotted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Slot", meta = (GameplayTagFilter="GameItemTagsCategory"))
	FGameplayTagQuery ItemQuery;

	/** Context tags to pass when auto-slotting the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Slot")
	FGameplayTagContainer ContextTags;

	virtual void OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer) override;

protected:
	void OnLinkedItemAdded(UGameItem* Item);
};
