// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerLink.h"
#include "GameItemContainerLink_Selection.generated.h"


/**
 * Updates this container to represent an item in a slot of the linked container.
 * Provides options for selecting the next or previous item, as well as ensuring
 * an item is always selected when possible.
 */
UCLASS(DisplayName = "Link (Selection)")
class GAMEITEMS_API UGameItemContainerLink_Selection : public UGameItemContainerLink
{
	GENERATED_BODY()

public:
	UGameItemContainerLink_Selection();

	/** The slot in this container where the selected item should be added. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Selection")
	int32 TargetSlot;

	/** If true, allow selecting slots in the linked container that have no item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Selection")
	bool bAllowSelectingEmptySlots;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetSelectedSlot(int32 NewSlot);

	/**
	 * Find the slot of an item in the linked container, and set the selected slot to that.
	 * Does nothing if the item isn't found.
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetSelectedItem(UGameItem* Item);

	/** Select the next slot, or the next item if empty slots cannot be selected. */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SelectNextSlot(bool bLoop = true);

	/** Select the previous slot, or the previous item if empty slots cannot be selected. */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SelectPrevSlot(bool bLoop = true);

	/** Select the next valid item in the linked container. */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SelectNextItem(bool bLoop = true);

	/** Select the previous valid item in the linked container. */
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SelectPrevItem(bool bLoop = true);

	/** Clamp or loop a slot in the linked container. */
	UFUNCTION(BlueprintPure)
	int32 ClampSlot(int32 Slot, bool bLoop) const;

	virtual void OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_SelectedSlot();

protected:
	/** The currently selected slot in the linked container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_SelectedSlot, Category = "Selection")
	int32 SelectedSlot;

	/** Find the next or previous slot in the linked container with a valid item. */
	int32 FindValidItemSlot(int32 SearchDirection, bool bLoop) const;

	void OnLinkedSlotChanged(int32 Slot);
	void OnLinkedItemAdded(UGameItem* GameItem);

	/** Update this container to contain the item from the selected slot in the linked container. */
	virtual void UpdateContainerForSelection();
};
