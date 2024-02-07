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

	UGameItemContainer* GetContainer() const { return Container; }

	int32 GetSlot() const { return Slot; }

	/** Does the slot have an item in it? */
	UFUNCTION(BlueprintPure, FieldNotify)
	bool HasItem() const;

	/** Is this a valid slot in the container? */
	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsValidSlot() const;

	/** Return the current item. */
	UGameItem* GetItem() const { return Item; }

	/** Move the item in this slot to a new container. */
	UFUNCTION(BlueprintCallable)
	TArray<UGameItem*> MoveItem(UGameItemContainer* ToContainer, bool bAllowPartial = true);

	virtual UWorld* GetWorld() const override;

protected:
	/** The owning container. */
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	UGameItemContainer* Container;

	/** The slot in the container. */
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	int32 Slot;

	/** The currently slotted item. */
	UPROPERTY(Transient, BlueprintReadOnly, FieldNotify)
	UGameItem* Item;

	void UpdateItem();

	void OnItemSlotChanged(int32 InSlot);
	void OnItemSlotsChanged(int32 StartSlot, int32 EndSlot);
	void OnNumSlotsChanged(int32 NewNumSlots, int32 OldNumSlots);

public:
	/** Create an slot view model for each slot in a container, which can be provided to a list view. */
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf = "Outer"))
	static TArray<UGameItemContainerSlotViewModel*> CreateSlotViewModelsForContainer(UObject* Outer, UGameItemContainer* InContainer);
};
