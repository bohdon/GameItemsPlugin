// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerLink.h"

#include "GameItemContainer.h"
#include "GameItemsModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerLink)


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
