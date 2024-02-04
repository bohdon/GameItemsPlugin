// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DemoInteraction.h"
#include "EnhancedInputComponent.h"
#include "Components/SceneComponent.h"
#include "DemoInteractionComponent.generated.h"

class UDemoInteraction;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEITEMSPLUGIN_API UDemoInteractionComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UDemoInteractionComponent();

	/** Maximum distance at which this interaction can be focused. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance;

	/** The available interactions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<TObjectPtr<UDemoInteraction>> Interactions;

	/** The UI component to use for displaying interactions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (MustImplement = "DemoInteractionUIInterface"))
	TSubclassOf<USceneComponent> UIComponentClass;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFocusReceived(APawn* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFocusLost(APawn* Interactor);

	UFUNCTION(BlueprintCallable)
	void ShowInteractUI();

	UFUNCTION(BlueprintCallable)
	void HideInteractUI();

	UFUNCTION(BlueprintCallable)
	void BindInteractionInput(APawn* Interactor);

	UFUNCTION(BlueprintCallable)
	void UnbindInteractionInput(APawn* Interactor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void TriggerInteraction(APawn* Interactor, UDemoInteraction* Interaction);

	virtual void Activate(bool bReset) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> UIComponent;

	/** Active input bindings, by interactor. */
	TMap<APawn*, TArray<FInputBindingHandle>> InputBindings;

	void CreateInteractUI();

	void InteractionInputPressed(APawn* Interactor, UDemoInteraction* Interaction);
};
