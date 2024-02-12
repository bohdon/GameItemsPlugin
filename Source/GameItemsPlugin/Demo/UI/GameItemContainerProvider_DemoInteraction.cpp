// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerProvider_DemoInteraction.h"

#include "GameItemStatics.h"
#include "Blueprint/UserWidget.h"
#include "Demo/Interaction/DemoInteractionComponent.h"
#include "Demo/Interaction/DemoInteractionTypes.h"
#include "Demo/Interaction/DemoInteractorComponent.h"
#include "GameFramework/Pawn.h"


UGameItemContainer* UGameItemContainerProvider_DemoInteraction::ProvideContainer_Implementation(const FGameplayTag& ContainerId,
                                                                                                    const FGameItemViewContext& ContextData) const
{
	const FDemoInteractionContext InteractionContext = UDemoInteractorComponent::GetInteractionContextForActor(ContextData.UserWidget->GetOwningPlayerPawn());
	const UDemoInteractionComponent* InteractionComp = InteractionContext.InteractionComponent.Get();
	return UGameItemStatics::GetItemContainerForActor(InteractionComp ? InteractionComp->GetOwner() : nullptr, ContainerId);
}
