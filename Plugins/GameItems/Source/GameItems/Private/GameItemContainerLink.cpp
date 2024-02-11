// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerLink.h"

#include "GameItemContainer.h"
#include "GameItemsModule.h"


// UGameItemContainerLink
// ----------------------

void UGameItemContainerLink::SetLinkedContainer(UGameItemContainer* NewContainer)
{
	if (LinkedContainer != NewContainer)
	{
		if (Container && NewContainer && NewContainer == Container)
		{
			UE_LOG(LogGameItems, Warning, TEXT("UGameItemContainerLink: Cant link a container to itself. Use UGameItemContainerRules instead. "
				       "%s -> %s"), *Container->ContainerId.ToString(), *NewContainer->ContainerId.ToString());
			return;
		}

		UGameItemContainer* OldContainer = LinkedContainer;
		LinkedContainer = NewContainer;

		OnLinkedContainerChanged(LinkedContainer, OldContainer);
	}
}

void UGameItemContainerLink::OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer)
{
}


// UGameItemContainerLink_Parent
// -----------------------------

void UGameItemContainerLink_Parent::OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer)
{
	Super::OnLinkedContainerChanged(NewContainer, OldContainer);

	if (OldContainer)
	{
		OldContainer->OnItemRemovedEvent.RemoveAll(this);
	}

	if (NewContainer)
	{
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
