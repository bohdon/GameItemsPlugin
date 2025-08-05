// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainer.h"
#include "MVVMViewModelBase.h"
#include "VM_GameItemSlot.generated.h"

class UGameItem;
class UGameItemContainer;


/**
 * A view-model for a specific slot in a game item container.
 */
UCLASS()
class GAMEITEMSUI_API UVM_GameItemSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UVM_GameItemSlot();

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
	UPROPERTY(BlueprintReadOnly, Getter, FieldNotify, Meta = (AllowPrivateAccess))
	UGameItemContainer* Container;

	/** The slot in the container. */
	UPROPERTY(BlueprintReadOnly, Getter, FieldNotify, Meta = (AllowPrivateAccess))
	int32 Slot;

	/** The currently slotted item. */
	UPROPERTY(BlueprintReadOnly, Getter, FieldNotify)
	UGameItem* Item;

	void UpdateItem();

	void OnItemSlotChanged(int32 InSlot);
	void OnItemSlotsChanged(int32 StartSlot, int32 EndSlot);
	void OnNumSlotsChanged(int32 NewNumSlots, int32 OldNumSlots);

public:
	/** Create a slot view model for each slot in a container, which can be provided to a list view. */
	UFUNCTION(BlueprintCallable, Meta = (DefaultToSelf = "Outer"))
	static TArray<UVM_GameItemSlot*> CreateSlotViewModelsForContainer(UObject* Outer, UGameItemContainer* InContainer);
};
