// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "GameItemContainerLink_Parent.h"
#include "GameItemContainerLink_AutoSlot.generated.h"


/**
 * Tries to auto-slot an item in this container when it's added to the linked container.
 * By default, this rule includes the 'Item.AutoSlot.NoReplace' context tag when auto-slotting.
 */
UCLASS(DisplayName = "Link (Trigger Auto Slot)")
class UGameItemContainerLink_AutoSlot : public UGameItemContainerLink_Parent
{
	GENERATED_BODY()

public:
	UGameItemContainerLink_AutoSlot();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Items must match this query to be auto-slotted. If empty, all compatible items are auto-slotted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Auto Slot", meta = (GameplayTagFilter="GameItemTagsCategory"))
	FGameplayTagQuery ItemQuery;

	/** Context tags to pass when auto-slotting the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Auto Slot")
	FGameplayTagContainer ContextTags;

	/** If true, allow triggering auto-slotting even when loading save data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Auto Slot")
	bool bAutoSlotDuringLoad = false;

protected:
	virtual void OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer) override;

	void OnLinkedItemAdded(UGameItem* Item);
};
