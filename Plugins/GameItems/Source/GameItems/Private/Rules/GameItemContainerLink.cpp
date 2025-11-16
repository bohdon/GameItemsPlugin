// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerLink.h"

#include "GameItemContainer.h"
#include "GameItemContainerInterface.h"
#include "GameItemsModule.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerLink)


void UGameItemContainerLink::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerLink, LinkedContainerId, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerLink, LinkedContainer, Params);
}

FString UGameItemContainerLink::GetDebugString() const
{
	return FString::Printf(TEXT("%s (%s%s)"),
	                       *Super::GetDebugString(),
	                       *LinkedContainerId.ToString(),
	                       IsValid(LinkedContainer) ? TEXT("") : TEXT("..."));
}

void UGameItemContainerLink::SetLinkedContainer(UGameItemContainer* NewContainer)
{
	if (LinkedContainer != NewContainer)
	{
		if (NewContainer)
		{
			const UGameItemContainer* Container = GetContainer();
			check(Container);
			if (NewContainer == Container)
			{
				UE_LOG(LogGameItems, Warning, TEXT("[%hs] Cant link container to itself: %s (%s)"),
				       __FUNCTION__, *Container->GetReadableName(), *GetName());
				return;
			}

			// update container id if this is totally new (not just a resolve)
			if (LinkedContainerId != NewContainer->GetContainerId())
			{
				LinkedContainerId = NewContainer->GetContainerId();
				MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LinkedContainerId, this);
			}
		}

		UGameItemContainer* OldContainer = LinkedContainer;
		LinkedContainer = NewContainer;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LinkedContainer, this);

		OnLinkedContainerChanged(LinkedContainer, OldContainer);
	}
}

void UGameItemContainerLink::ResolveLinkedContainer(const IGameItemContainerInterface* ContainerProvider, bool bForce)
{
	if (!ensure(ContainerProvider))
	{
		return;
	}

	if (LinkedContainerId.IsValid() && (bForce || !LinkedContainer))
	{
		if (UGameItemContainer* Container = ContainerProvider->GetItemContainer(LinkedContainerId))
		{
			SetLinkedContainer(Container);
		}
		else
		{
			UE_LOG(LogGameItems, Verbose, TEXT("Failed to resolve link: %s.%s (%s)"),
			       *GetContainer()->GetReadableName(), *GetName(), *LinkedContainerId.ToString());
		}
	}
}

void UGameItemContainerLink::OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer)
{
}
