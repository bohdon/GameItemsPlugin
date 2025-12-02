// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoCharacter.h"

#include "EnhancedInputComponent.h"
#include "GameItemStatics.h"
#include "ItemsDemoPlayerState.h"
#include "Demo/Interaction/DemoInteractorComponent.h"
#include "Equipment/GameItemEquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"


AItemsDemoCharacter::AItemsDemoCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractorComponent = CreateDefaultSubobject<UDemoInteractorComponent>(TEXT("InteractorComponent"));
	InteractorComponent->SetupAttachment(RootComponent);

	EquipmentComponent = CreateDefaultSubobject<UGameItemEquipmentComponent>(TEXT("EquipmentComponent"));
	// register item component once we have a player state (where it's stored)
	EquipmentComponent->bAutoFindContainerComponent = false;
}

void AItemsDemoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	EnhancedInput->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AItemsDemoCharacter::InputMove);
	EnhancedInput->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &AItemsDemoCharacter::InputLook);
}

void AItemsDemoCharacter::InputMove(const FInputActionValue& InputActionValue)
{
	const FVector2D Input = InputActionValue.Get<FVector2D>();
	const FVector WorldInput = GetControlRotation().RotateVector(FVector(Input, 0.f)).GetSafeNormal2D();
	GetCharacterMovement()->AddInputVector(WorldInput);
}

void AItemsDemoCharacter::InputLook(const FInputActionValue& InputActionValue)
{
	const FVector2D Input = InputActionValue.Get<FVector2D>();
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

TArray<UGameItemContainer*> AItemsDemoCharacter::GetAllItemContainers() const
{
	// use the item containers from the player state
	if (const AItemsDemoPlayerState* DemoPlayerState = GetPlayerState<AItemsDemoPlayerState>())
	{
		return DemoPlayerState->GetAllItemContainers();
	}
	return TArray<UGameItemContainer*>();
}

UGameItemContainer* AItemsDemoCharacter::GetItemContainer(FGameplayTag ContainerId) const
{
	return UGameItemStatics::GetItemContainerForActor(GetPlayerState(), ContainerId);
}

UGameItemContainerComponent* AItemsDemoCharacter::GetItemContainerComponent() const
{
	return UGameItemStatics::GetItemContainerComponentForActor(GetPlayerState());
}

void AItemsDemoCharacter::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

	// now that we have a player state, we have an item component
	if (UGameItemContainerComponent* ItemComponent = GetItemContainerComponent())
	{
		EquipmentComponent->RegisterItemContainerComponent(ItemComponent);
	}
}
