// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "GameItemContainerLink.h"
#include "GameItemContainerLink_AutoSlot.generated.h"


/**
 * Auto-slot an item in this container when added to the linked container.
 */
UCLASS(DisplayName = "Link Auto-Slot")
class UGameItemContainerLink_AutoSlot : public UGameItemContainerLink
{
	GENERATED_BODY()

public:
	/** Items must match this query to be auto-slotted. If empty, all compatible items are auto-slotted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Slot")
	FGameplayTagQuery ItemQuery;

	virtual void OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer) override;

protected:
	void OnLinkedItemAdded(UGameItem* Item);
};
