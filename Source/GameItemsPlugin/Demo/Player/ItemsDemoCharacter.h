// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerComponentInterface.h"
#include "GameItemContainerInterface.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "ItemsDemoCharacter.generated.h"

class UDemoInteractorComponent;
class UGameItemEquipmentComponent;


UCLASS()
class GAMEITEMSPLUGIN_API AItemsDemoCharacter : public ACharacter,
                                                public IGameItemContainerInterface,
                                                public IGameItemContainerComponentInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDemoInteractorComponent> InteractorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGameItemEquipmentComponent> EquipmentComponent;

public:
	AItemsDemoCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void InputMove(const FInputActionValue& InputActionValue);
	void InputLook(const FInputActionValue& InputActionValue);

	// IGameItemContainerInterface
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const override;
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const override;

	// IGameItemContainerComponentInterface
	virtual UGameItemContainerComponent* GetItemContainerComponent() const override;

protected:
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UInputAction> MoveInputAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UInputAction> LookInputAction;
};
