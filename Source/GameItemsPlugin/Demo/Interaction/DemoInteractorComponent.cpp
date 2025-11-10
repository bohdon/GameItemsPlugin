// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoInteractorComponent.h"

#include "DemoInteractionComponent.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Slate/SceneViewport.h"
#include "UObject/UObjectIterator.h"

TAutoConsoleVariable CVarDebugDemoInteractions(
	TEXT("game.interactions.Debug"),
	false,
	TEXT("Draw debug for interactions"));


UDemoInteractorComponent::UDemoInteractorComponent()
	: MaxFocusAngle(60.f)
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
}

void UDemoInteractorComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	GetWorld()->GetTimerManager().SetTimer(LatentUpdateTimer, this, &UDemoInteractorComponent::LatentUpdateInteractions, 5.f, true);
	LatentUpdateInteractions();
}

void UDemoInteractorComponent::Deactivate()
{
	Super::Deactivate();

	GetWorld()->GetTimerManager().ClearTimer(LatentUpdateTimer);
	SetFocusedInteraction(nullptr);
}

void UDemoInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsActive())
	{
		return;
	}

	SetFocusedInteraction(GetBestInteraction());
}

void UDemoInteractorComponent::SetContext(FDemoInteractionContext NewInteractionContext)
{
	Context = NewInteractionContext;
}

void UDemoInteractorComponent::LatentUpdateInteractions()
{
	NearbyInteractions.Reset();
	const UWorld* World = GetWorld();
	for (TObjectIterator<UDemoInteractionComponent> It; It; ++It)
	{
		UDemoInteractionComponent* Interaction = *It;
		if (Interaction->GetWorld() == World)
		{
			NearbyInteractions.Add(Interaction);
		}
	}
}

FTransform UDemoInteractorComponent::GetAimTransform() const
{
	if (const APawn* Pawn = GetOwner<APawn>())
	{
		if (const APlayerController* Player = Pawn->GetController<APlayerController>())
		{
			const FVector Location = Player->PlayerCameraManager->GetCameraLocation();
			const FRotator Rotation = Player->PlayerCameraManager->GetCameraRotation();
			return FTransform(Rotation, Location);
		}
	}
	return GetComponentTransform();
}

bool UDemoInteractorComponent::CanInteract() const
{
	UGameViewportClient* ViewportClient = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	if (ViewportClient && !ViewportClient->GetGameViewport()->HasMouseCapture())
	{
		// ignore input when not capturing mouse
		return false;
	}
	return true;
}

UDemoInteractionComponent* UDemoInteractorComponent::GetBestInteraction() const
{
	if (!CanInteract())
	{
		return nullptr;
	}

	const FTransform AimTransform = GetAimTransform();
	const FVector AimDir = AimTransform.GetRotation().Vector();
	const FVector AimLocation = GetComponentLocation();

	UDemoInteractionComponent* BestInteraction = nullptr;
	float BestAngle = 180.f;

	for (const TWeakObjectPtr<UDemoInteractionComponent>& Interaction : NearbyInteractions)
	{
		if (!Interaction.IsValid())
		{
			continue;
		}

		const FTransform Transform = Interaction->GetComponentTransform();
		const FVector Delta = Transform.GetLocation() - AimLocation;
		const FVector DeltaDir = Delta.GetSafeNormal();
		const float ViewAngle = GetDeltaAngle(AimDir, DeltaDir);
		const float DistSq = Delta.SquaredLength();
		const bool bCanFocus = ViewAngle <= MaxFocusAngle && DistSq <= FMath::Square(Interaction->MaxDistance);

#if ENABLE_DRAW_DEBUG
		if (CVarDebugDemoInteractions.GetValueOnAnyThread())
		{
			FColor DebugColor = bCanFocus ? FColor::Cyan : FColor::Red;
			DrawDebugString(GetWorld(), Interaction->GetComponentLocation(),
			                FString::Printf(TEXT("Angle: %f, Dist: %f"), ViewAngle, FMath::Sqrt(DistSq)),
			                nullptr, DebugColor, 0.f);
		}
#endif

		if (!bCanFocus)
		{
			continue;
		}

		if (ViewAngle < BestAngle)
		{
			BestInteraction = Interaction.Get();
			BestAngle = ViewAngle;
		}
	}

	return BestInteraction;
}

void UDemoInteractorComponent::SetFocusedInteraction(UDemoInteractionComponent* Interaction)
{
	if (Interaction == FocusedInteraction.Get())
	{
		return;
	}

	if (FocusedInteraction.IsValid())
	{
		FocusedInteraction->OnFocusLost(GetOwner<APawn>());
	}

	FocusedInteraction = Interaction;

	if (FocusedInteraction.IsValid())
	{
		FocusedInteraction->OnFocusReceived(GetOwner<APawn>());
	}
}

float UDemoInteractorComponent::GetDeltaAngle(const FVector& DirectionA, const FVector& DirectionB)
{
	const float Dot = DirectionA.GetSafeNormal() | DirectionB.GetSafeNormal();
	return FMath::RadiansToDegrees(FMath::Acos(Dot));
}

FDemoInteractionContext UDemoInteractorComponent::GetInteractionContextForActor(AActor* Actor)
{
	if (const UDemoInteractorComponent* InteractorComp = Actor->FindComponentByClass<UDemoInteractorComponent>())
	{
		return InteractorComp->GetContext();
	}
	return FDemoInteractionContext();
}
