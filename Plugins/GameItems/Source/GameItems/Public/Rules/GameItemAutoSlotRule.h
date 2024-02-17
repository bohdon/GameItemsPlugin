// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainer.h"
#include "UObject/Object.h"
#include "GameItemAutoSlotRule.generated.h"

class UGameItem;
class UGameItemContainer;


/**
 * Defines logic for automatically slotting items into containers.
 *
 * Auto-slotting can be used for variety of actions, often synonymous with 'equipping':
 *	- When a user right-clicks an item, 'equip' it to a matching slot
 *	- When an item is picked up, 'auto-equip' it to a matching slot if that slot is empty
 */
UCLASS(Abstract, DefaultToInstanced, EditInlineNew)
class GAMEITEMS_API UGameItemAutoSlotRule : public UObject
{
	GENERATED_BODY()

public:
	/** Attempt to auto-slot an item to a container, and return the added items if auto-slotting was successful. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = false)
	TArray<UGameItem*> TryAutoSlotItem(UGameItem* Item, const TArray<UGameItemContainer*>& Containers, const FGameplayTagContainer& ContextTags) const;

protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = false)
	TArray<UGameItem*> AutoSlotItem(UGameItem* Item, UGameItemContainer* Container, int32 Slot, bool bShouldReplace) const;
};


/**
 * Auto-slots items to a specific container based on the item tags.
 * Replaces existing items based on the AutoSlotTags.
 */
UCLASS(DisplayName = "Item Tags")
class UGameItemAutoSlotRule_ItemTags : public UGameItemAutoSlotRule
{
	GENERATED_BODY()

public:
	/** Match items that have any of these tags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Tags")
	FGameplayTagContainer ItemTags;

	/** The container to auto-slot the item to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Tags")
	FGameplayTag ContainerId;

	/** If ContextTags matches any of these tags, replace existing items in the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Tags")
	FGameplayTagContainer ReplaceContextTags;

	virtual TArray<UGameItem*> TryAutoSlotItem_Implementation(UGameItem* Item, const TArray<UGameItemContainer*>& Containers,
	                                                          const FGameplayTagContainer& ContextTags) const override;
};
