// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainer.h"
#include "MVVMViewModelBase.h"
#include "VM_GameItemSlottedCheck.generated.h"

class UGameItem;


/**
 * View model that checks whether an item is slotted into specific containers by tag.
 * This is most commonly used to check if an item is 'equipped'.
 */
UCLASS()
class GAMEITEMSUI_API UVM_GameItemSlottedCheck : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void SetContainerTags(FGameplayTagContainer NewContainerTags);

	const FGameplayTagContainer& GetContainerTags() const { return ContainerTags; }

	/** Set the game item for this view model .*/
	void SetItem(UGameItem* NewItem);

	/** Return the current item. */
	UGameItem* GetItem() const { return Item; }

	/** Return true if the item is slotted in a container with any of ContainerTags. */
	UFUNCTION(BlueprintPure, FieldNotify)
	bool IsSlotted() const;

protected:
	/** If the item is slotted to a container with any of these tags, consider it equipped. */
	UPROPERTY(BlueprintReadWrite, Setter, Getter, FieldNotify, Meta = (AllowPrivateAccess))
	FGameplayTagContainer ContainerTags;

	/** The game item. */
	UPROPERTY(BlueprintReadWrite, Setter, Getter, FieldNotify, Meta = (AllowPrivateAccess))
	TObjectPtr<UGameItem> Item;

	void OnSlotted(const UGameItemContainer* Container, int32 NewSlot, int32 OldSlot);
	void OnUnslotted(const UGameItemContainer* Container, int32 OldSlot);
};
