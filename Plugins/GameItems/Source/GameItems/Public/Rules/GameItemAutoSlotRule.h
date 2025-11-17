// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerRule.h"
#include "GameItemAutoSlotRule.generated.h"

class UGameItem;


/**
 * Defines logic for automatically slotting items into containers.
 * Specifically, this rule selects the best slot for an item, and can report the priority
 * of this container when selecting the 'best' container for an item.
 *
 * Auto-slotting can be used for variety of actions, often synonymous with 'equipping':
 *	- When a user right-clicks an item, equip it to the best slot
 *	- Auto-equip items when picked up, but only if nothing is already equipped
 */
UCLASS(Abstract)
class GAMEITEMS_API UGameItemAutoSlotRule : public UGameItemContainerRule
{
	GENERATED_BODY()

public:
	/** Return the priority of this container when selecting the 'best' container for auto-slotting an item. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	int32 GetAutoSlotPriorityForItem(UGameItem* Item, const FGameplayTagContainer& ContextTags) const;

	/** Return true if this rule can handle auto-slotting an item. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	bool CanAutoSlot(UGameItem* Item, const FGameplayTagContainer& ContextTags) const;

	/**
	 * Try auto-slotting an item.
	 * @return True if the auto-slotting was successful. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	void TryAutoSlot(UGameItem* Item, const FGameplayTagContainer& ContextTags) const;

	/** Return the best slot to use for auto-slotting an item. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	int32 GetBestSlotForItem(UGameItem* Item, const FGameplayTagContainer& ContextTags) const;

	/** Return true if an item should replace an existing item in the container. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	bool ShouldReplaceItem(UGameItem* NewItem, UGameItem* ExistingItem, const FGameplayTagContainer& ContextTags) const;

public:
	UFUNCTION(Server, Reliable)
	virtual void ServerTryAutoSlot(UGameItem* Item, const FGameplayTagContainer& ContextTags) const;
};
