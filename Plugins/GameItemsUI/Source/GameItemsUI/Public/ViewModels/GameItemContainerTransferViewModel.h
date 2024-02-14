// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "GameItemContainerTransferViewModel.generated.h"

class UGameItemContainer;
class UGameItemSlotViewModel;


/**
 * A view model to help with common transactions between two containers.
 */
UCLASS()
class GAMEITEMSUI_API UGameItemContainerTransferViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	virtual void SetContainerA(UGameItemContainer* NewContainer);

	UGameItemContainer* GetContainerA() const { return ContainerA; }

	virtual void SetContainerB(UGameItemContainer* NewContainer);

	UGameItemContainer* GetContainerB() const { return ContainerB; }

	/** Return true if both ContainerA and ContainerB are valid. */
	bool AreContainersValid() const { return ContainerA && ContainerB; }

	/** Move an item from one container to the other. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveItem(UGameItemSlotViewModel* SlotViewModel, bool bAllowPartial = true);

	/** Move all items from container A to B. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveAllItemsToA(bool bAllowPartial = true);

	/** Move all items from container B to A. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveAllItemsToB(bool bAllowPartial = true);

protected:
	UPROPERTY(Transient, BlueprintReadWrite, Setter, Getter, FieldNotify)
	UGameItemContainer* ContainerA;

	UPROPERTY(Transient, BlueprintReadWrite, Setter, Getter, FieldNotify)
	UGameItemContainer* ContainerB;
};
