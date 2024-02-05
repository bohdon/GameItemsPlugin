// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainer.h"
#include "MVVMViewModelBase.h"
#include "GameItemContainerSlotViewModel.generated.h"

class UGameItem;
class UGameItemContainer;


/**
 * A view-model for a specific slot in a game item container.
 */
UCLASS()
class GAMEITEMSUI_API UGameItemContainerSlotViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UGameItemContainerSlotViewModel();

	/** Set the container and slot to use, and update the item accordingly. */
	UFUNCTION(BlueprintCallable)
	void SetContainerAndSlot(UGameItemContainer* NewContainer, int32 NewSlot);

	/** Does the slot have an item in it? */
	UFUNCTION(BlueprintPure, FieldNotify)
	bool HasItem() const;

protected:
	/** The owning container. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify)
	UGameItemContainer* Container;

	/** The slot in the container. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify)
	int32 Slot;

	/** The currently slotted item. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, FieldNotify)
	UGameItem* Item;

	void UpdateItem();

public:
	/** Create an slot view model for each slot in a container, which can be provided to a list view. */
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf = "Outer"))
	static TArray<UGameItemContainerSlotViewModel*> CreateSlotViewModelsForContainer(UObject* Outer, UGameItemContainer* InContainer);
};
