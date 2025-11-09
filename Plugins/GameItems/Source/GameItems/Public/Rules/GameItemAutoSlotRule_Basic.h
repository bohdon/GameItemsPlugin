// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemAutoSlotRule.h"
#include "NativeGameplayTags.h"
#include "GameItemAutoSlotRule_Basic.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_AutoSlot_NoReplace);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_AutoSlot_Replace);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Item_AutoSlot_Toggle);


/**
 * A basic auto-slot rule that looks for the next-empty slot, or uses the first slot
 * if the container has only one slot.
 *
 * Handles some specific context tags to alter behavior:
 *	- 'Item.AutoSlot.NoReplace' - Don't replace existing items in the target slot (if bReplaceByDefault is enabled)
 *	- 'Item.AutoSlot.Replace' - Replace existing items in the target slot (if bReplaceByDefault is disabled)
 *	- 'Item.AutoSlot.Toggle' - Remove the item instead of slotting it, if it's already in the container.
 */
UCLASS(DisplayName = "Auto Slot (Basic)")
class GAMEITEMS_API UGameItemAutoSlotRule_Basic : public UGameItemAutoSlotRule
{
	GENERATED_BODY()

public:
	UGameItemAutoSlotRule_Basic();

	/** The base priority of the container when being selected for auto-slotting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Auto Slot")
	int32 Priority;

	/**
	 * Should existing items be replaced by default?
	 * 'Item.AutoSlot.NoReplace' or 'Item.AutoSlot.Replace' context tags can override this setting.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Auto Slot")
	bool bReplaceByDefault;

	/** Items must have all of these tags to be auto slotted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Tag Requirements", meta = (GameplayTagFilter="GameItemTagsCategory"))
	FGameplayTagContainer RequireTags;

	/** Items cannot have any of these tags to be auto slotted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Tag Requirements", meta = (GameplayTagFilter="GameItemTagsCategory"))
	FGameplayTagContainer IgnoreTags;

	/** Items must match this query to be auto slotted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Tag Requirements", meta = (GameplayTagFilter="GameItemTagsCategory"))
	FGameplayTagQuery Query;

	virtual bool CanAutoSlot_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const override;
	virtual int32 GetAutoSlotPriorityForItem_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const override;
	virtual bool TryAutoSlot_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags, TArray<UGameItem*>& OutItems) const override;
	virtual int32 GetBestSlotForItem_Implementation(UGameItem* Item, const FGameplayTagContainer& ContextTags) const override;
	virtual bool ShouldReplaceItem_Implementation(UGameItem* NewItem, UGameItem* ExistingItem, const FGameplayTagContainer& ContextTags) const override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
