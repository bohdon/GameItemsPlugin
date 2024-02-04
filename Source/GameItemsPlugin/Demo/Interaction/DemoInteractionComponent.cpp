// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoInteractionComponent.h"

#include "DemoInteractionUIInterface.h"
#include "DemoInteractorComponent.h"
#include "EnhancedInputComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"


UDemoInteractionComponent::UDemoInteractionComponent()
	: MaxDistance(250.f)
{
	bAutoActivate = true;
}

void UDemoInteractionComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		CreateInteractUI();
	}
}

void UDemoInteractionComponent::OnFocusReceived_Implementation(APawn* Interactor)
{
	ShowInteractUI();
	BindInteractionInput(Interactor);
}

void UDemoInteractionComponent::OnFocusLost_Implementation(APawn* Interactor)
{
	HideInteractUI();
	UnbindInteractionInput(Interactor);
}

void UDemoInteractionComponent::ShowInteractUI()
{
	if (UIComponent)
	{
		IDemoInteractionUIInterface::Execute_Show(UIComponent, Interactions);
	}
}

void UDemoInteractionComponent::HideInteractUI()
{
	if (UIComponent)
	{
		IDemoInteractionUIInterface::Execute_Hide(UIComponent);
	}
}

void UDemoInteractionComponent::BindInteractionInput(APawn* Interactor)
{
	UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(Interactor->InputComponent);
	if (!InputComponent)
	{
		return;
	}

	for (UDemoInteraction* Interaction : Interactions)
	{
		if (!Interaction || !Interaction->InputAction)
		{
			continue;
		}

		FInputBindingHandle Binding = FInputBindingHandle(InputComponent->BindAction(Interaction->InputAction, ETriggerEvent::Triggered, this,
		                                                                             &UDemoInteractionComponent::InteractionInputPressed, Interactor,
		                                                                             Interaction));

		// save binding for removal
		TArray<FInputBindingHandle>& PawnInputBindings = InputBindings.FindOrAdd(Interactor);
		PawnInputBindings.Add(Binding);
	}
}

void UDemoInteractionComponent::UnbindInteractionInput(APawn* Interactor)
{
	UEnhancedInputComponent* InputComponent = Cast<UEnhancedInputComponent>(Interactor->InputComponent);
	if (!InputComponent)
	{
		return;
	}

	if (InputBindings.Contains(Interactor))
	{
		// remove all bindings for this interactor
		const TArray<FInputBindingHandle>& PawnInputBindings = InputBindings[Interactor];
		for (const FInputBindingHandle& Binding : PawnInputBindings)
		{
			InputComponent->RemoveBinding(Binding);
		}
		InputBindings.Remove(Interactor);
	}
}

void UDemoInteractionComponent::TriggerInteraction_Implementation(APawn* Interactor, UDemoInteraction* Interaction)
{
	if (!Interactor || !Interaction)
	{
		return;
	}

	Interaction->Trigger(Interactor);
}

void UDemoInteractionComponent::CreateInteractUI()
{
	if (!UIComponentClass || UIComponent)
	{
		return;
	}

	// create the widget component and attach it
	UIComponent = NewObject<USceneComponent>(GetOwner(), UIComponentClass, NAME_None, RF_Transient);
	UIComponent->OnComponentCreated();
	UIComponent->SetupAttachment(this);
	UIComponent->RegisterComponent();

	check(UIComponent->Implements<UDemoInteractionUIInterface>());
}

void UDemoInteractionComponent::InteractionInputPressed(APawn* Interactor, UDemoInteraction* Interaction)
{
	TriggerInteraction(Interactor, Interaction);
}
