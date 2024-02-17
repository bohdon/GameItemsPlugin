// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerLink_Parent.h"

#include "GameItemContainer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerLink_Parent)


bool UGameItemContainerLink_Parent::IsChild_Implementation() const
{
	return true;
}

void UGameItemContainerLink_Parent::OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer)
{
	Super::OnLinkedContainerChanged(NewContainer, OldContainer);

	if (OldContainer)
	{
		OldContainer->UnregisterChild(GetContainer());
		OldContainer->OnItemRemovedEvent.RemoveAll(this);
	}

	if (NewContainer)
	{
		NewContainer->RegisterChild(GetContainer());
		NewContainer->OnItemRemovedEvent.AddUObject(this, &UGameItemContainerLink_Parent::OnLinkedItemRemoved);
	}
}

bool UGameItemContainerLink_Parent::CanContainItem_Implementation(const UGameItem* Item) const
{
	return LinkedContainer && LinkedContainer->Contains(Item);
}

void UGameItemContainerLink_Parent::OnLinkedItemRemoved(UGameItem* Item)
{
	// remove from this container when removed from parent
	Container->RemoveItem(Item);
}
