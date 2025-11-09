// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerLink.h"

#include "GameItemContainer.h"
#include "GameItemsModule.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerLink)


void UGameItemContainerLink::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerLink, LinkedContainerId, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerLink, LinkedContainer, SharedParams);
}

void UGameItemContainerLink::SetLinkedContainer(UGameItemContainer* NewContainer)
{
	if (LinkedContainer != NewContainer)
	{
		const UGameItemContainer* Container = GetContainer();
		check(Container);
		if (Container && NewContainer && NewContainer == Container)
		{
			UE_LOG(LogGameItems, Warning, TEXT("UGameItemContainerLink: Cant link a container to itself. Use UGameItemContainerRules instead. "
				       "%s -> %s"), *Container->ContainerId.ToString(), *NewContainer->ContainerId.ToString());
			return;
		}

		UGameItemContainer* OldContainer = LinkedContainer;
		LinkedContainer = NewContainer;
		MARK_PROPERTY_DIRTY_FROM_NAME(UGameItemContainerLink, LinkedContainer, this);

		OnLinkedContainerChanged(LinkedContainer, OldContainer);
	}
}

void UGameItemContainerLink::OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer)
{
}