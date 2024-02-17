// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerLink_AutoSlot.h"

#include "GameItemContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerLink_AutoSlot)


void UGameItemContainerLink_AutoSlot::OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer)
{
	Super::OnLinkedContainerChanged(NewContainer, OldContainer);

	if (OldContainer)
	{
		OldContainer->OnItemAddedEvent.RemoveAll(this);
	}

	if (NewContainer)
	{
		NewContainer->OnItemAddedEvent.AddUObject(this, &UGameItemContainerLink_AutoSlot::OnLinkedItemAdded);
	}
}

void UGameItemContainerLink_AutoSlot::OnLinkedItemAdded(UGameItem* Item)
{
	if (Container->CanContainItem(Item) && (ItemQuery.IsEmpty() || ItemQuery.Matches(Item->GetOwnedTags())))
	{
		const int32 Slot = Container->GetNextEmptySlot();
		if (Slot != INDEX_NONE)
		{
			Container->AddItem(Item, Slot);
		}
	}
}
