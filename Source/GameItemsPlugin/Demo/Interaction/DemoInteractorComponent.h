// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DemoInteractorComponent.generated.h"

class UDemoInteractionComponent;


/**
 * Component that allows a pawn to interact with DemoInteractionComponents.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEITEMSPLUGIN_API UDemoInteractorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDemoInteractorComponent();

	/** Maximum angle that the interaction must be within the pawn's aim direction to receive focus. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFocusAngle;

	virtual void Activate(bool bReset) override;
	virtual void Deactivate() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	FTimerHandle LatentUpdateTimer;

	/** All interactions in the area. */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UDemoInteractionComponent>> NearbyInteractions;

	/** The current focused interaction */
	UPROPERTY(Transient)
	TWeakObjectPtr<UDemoInteractionComponent> FocusedInteraction;

	/** Infrequent update to gather all nearby interactions. */
	void LatentUpdateInteractions();

	FTransform GetAimTransform() const;

	bool CanInteract() const;
	
	UDemoInteractionComponent* GetBestInteraction() const;

	/** Set the currently focused interaction. */
	void SetFocusedInteraction(UDemoInteractionComponent* Interaction);

public:
	/** Return the delta angle between two directions. */
	UFUNCTION(BlueprintPure)
	static float GetDeltaAngle(const FVector& DirectionA, const FVector& DirectionB);
};
