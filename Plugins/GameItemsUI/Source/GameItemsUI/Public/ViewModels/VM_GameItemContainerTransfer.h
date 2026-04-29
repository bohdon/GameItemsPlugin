// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VM_GameItemContainerTransfer.generated.h"

class UGameItemControllerComponent;
class APlayerController;
class UGameItemContainer;
class UGameItemsUISubsystem;
class UVM_GameItemSlot;


/**
 * A view model to help with common transactions between two containers.
 * Requires an owning player in order to route commands through the UGameItemsUISubsystem.
 */
UCLASS()
class GAMEITEMSUI_API UVM_GameItemContainerTransfer : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void SetOwningPlayer(APlayerController* Player);

	APlayerController* GetOwningPlayer() const { return OwningPlayer; }

	virtual void SetContainerA(UGameItemContainer* NewContainer);

	UGameItemContainer* GetContainerA() const { return ContainerA; }

	virtual void SetContainerB(UGameItemContainer* NewContainer);

	UGameItemContainer* GetContainerB() const { return ContainerB; }

	/** Return true if both ContainerA and ContainerB are valid. */
	bool AreContainersValid() const { return ContainerA && ContainerB; }

	/** Move an item from one container to the other. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveItem(UVM_GameItemSlot* SlotViewModel, bool bAllowPartial = true);

	/** Move all items from container A to B. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveAllItemsToA(bool bAllowPartial = true);

	/** Move all items from container B to A. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveAllItemsToB(bool bAllowPartial = true);

protected:
	virtual UGameItemsUISubsystem* GetItemsUISubsystem() const;

	virtual UGameItemControllerComponent* GetGameItemController() const;

protected:
	/** The game item controller used to perform network operations. */
	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<APlayerController> OwningPlayer;

	UPROPERTY(Transient, BlueprintReadWrite, Setter, Getter, FieldNotify)
	TObjectPtr<UGameItemContainer> ContainerA;

	UPROPERTY(Transient, BlueprintReadWrite, Setter, Getter, FieldNotify)
	TObjectPtr<UGameItemContainer> ContainerB;
};
