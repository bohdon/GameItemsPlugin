// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoInteractionItemContainerViewModelResolver.h"

#include "Blueprint/UserWidget.h"
#include "Demo/Interaction/DemoInteractionComponent.h"
#include "Demo/Interaction/DemoInteractionTypes.h"
#include "Demo/Interaction/DemoInteractorComponent.h"
#include "GameFramework/Pawn.h"


AActor* UDemoInteractionItemContainerViewModelResolver::GetContainerActor(const UClass* ExpectedType, const UUserWidget* UserWidget,
                                                                          const UMVVMView* View) const
{
	const FDemoInteractionContext Context = UDemoInteractorComponent::GetInteractionContextForActor(UserWidget->GetOwningPlayerPawn());
	const UDemoInteractionComponent* InteractionComp = Context.InteractionComponent.Get();
	return InteractionComp ? InteractionComp->GetOwner() : nullptr;
}
