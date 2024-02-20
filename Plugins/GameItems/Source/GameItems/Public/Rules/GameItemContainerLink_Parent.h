// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "GameItemContainerLink.h"
#include "GameItemContainerLink_Parent.generated.h"


/**
 * Requires an item exist in the linked container before it can be added to this one.
 * This link will also indicate that the container cannot store its own items, which affects how items are moved.
 */
UCLASS(DisplayName = "Link (Parent)")
class UGameItemContainerLink_Parent : public UGameItemContainerLink
{
	GENERATED_BODY()

public:
	virtual bool IsChild_Implementation() const override;
	virtual void OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer) override;
	virtual bool CanContainItem_Implementation(const UGameItem* Item) const override;

protected:
	void OnLinkedItemRemoved(UGameItem* Item);
};
