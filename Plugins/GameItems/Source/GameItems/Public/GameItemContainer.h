// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItem.h"
#include "GameItemTypes.h"
#include "UObject/Object.h"
#include "GameItemContainer.generated.h"

class UGameItem;
class UGameItemContainerDef;
class UGameItemContainerStockRule;
class UGameItemDef;
class UGameItemSet;


/**
 * Component that contains one or more game item instances,
 * like an inventory, treasure chest, or just a simple item pickup.
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class GAMEITEMS_API UGameItemContainer : public UObject
{
	GENERATED_BODY()

public:
	UGameItemContainer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** The unique tag identifying this container amongst others. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItemContainer")
	FGameplayTag IdTag;

	/** The settings and rules for this container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItemContainer")
	TSubclassOf<UGameItemContainerDef> ContainerDef;

	/** Return the CDO of the container definition. */
	FORCEINLINE const UGameItemContainerDef* GetContainerDefCDO() const;

	/** Return true if a new item could be added to this container. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Meta = (DeprecatedFunction), Category = "GameItemContainer")
	bool CanAddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/**
	 * Add one or more of a new item to this container.
	 * @return The newly created item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Meta = (DeprecatedFunction), Category = "GameItemContainer")
	UGameItem* AddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/**
	 * Add an item to this container.
	 * @param Item The item to add.
	 * @param TargetSlot The slot where the item should be added, or the first available if -1.
	 * @return The item or items (if split into multiple stacks) that were added.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItemContainer")
	TArray<UGameItem*> AddItem(UGameItem* Item, int32 TargetSlot = -1);

	/** Remove an item from this container. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItemContainer")
	void RemoveItem(UGameItem* Item);

	/** Remove an item from a specific slot of this container. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItemContainer")
	UGameItem* RemoveItemAt(int32 Slot);

	/** Return all items in the container. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	TArray<UGameItem*> GetAllItems() const;

	/** Return the item in a specific slot of this container. */
	UFUNCTION(BlueprintCallable, Category = "GameItemContainer")
	UGameItem* GetItemAt(int32 Slot) const;

	/** Return the first stack of an item by definition. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	UGameItem* FindFirstItemByDef(TSubclassOf<UGameItemDef> ItemDef) const;

	/**
	 * Return the first stack of an item that matches another item.
	 * See UGameItem::IsMatching.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	UGameItem* FindFirstMatchingItem(const UGameItem* Item) const;

	/**
	 * Return all items that match another item.
	 * See UGameItem::IsMatching.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	TArray<UGameItem*> GetAllMatchingItems(const UGameItem* Item) const;

	/** Return the slot of an item in this container, or -1 if not in the container. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	int32 GetItemSlot(UGameItem* Item) const;

	/** Return the total number of an item in this container by definition. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	int32 GetTotalItemCountByDef(TSubclassOf<UGameItemDef> ItemDef) const;

	/**
	 * Return the total number of matching items in this container, including stack quantities.
	 * See UGameItem::IsMatching.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	int32 GetTotalMatchingItemCount(const UGameItem* Item) const;

	/** Return the total number of all items, including stack quantities. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	int32 GetTotalItemCount() const;

	/** Return the number of items (stacks) in this container. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	int32 GetNumItems() const;

	/** Return the number of empty slots available in this container, or -1 if unlimited. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	int32 GetNumEmptySlots() const;

	/** Return the next available empty slot in this container, or -1 if no slots are available. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	int32 GetNextEmptySlot() const;

	/** Return true if a slot in this container is empty and available. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	bool IsSlotEmpty(int32 Slot) const;

	/** Return the maximum total number of an item allowed in this container (for all stacks combined). */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	int32 GetItemMaxCount(const UGameItem* Item) const;

	/** Return the maximum count for a single stack of an item in this container. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	int32 GetItemStackMaxCount(const UGameItem* Item) const;

	/**
	 * Add the default items defined for this container.
	 * @param bForce If true, add the items even if they had previously been added.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddDefaultItems(bool bForce = false);

	/** Return the owning actor of this container. */
	virtual AActor* GetOwner() const;

	virtual UWorld* GetWorld() const override;

	DECLARE_MULTICAST_DELEGATE_OneParam(FItemAddOrRemoveDelegate, UGameItem* /*Item*/);

	/** Called when a new item is added. */
	FItemAddOrRemoveDelegate OnItemAddedEvent;

	/** Called when an item is removed. */
	FItemAddOrRemoveDelegate OnItemRemovedEvent;

protected:
	/** Have the default items already been added to this container? */
	bool bHasDefaultItems;

	/** Create and return a new item instance using the GameItemSubsystem. */
	UGameItem* CreateItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count) const;

	/** Duplicate and return a new item instance using the GameItemSubsystem. */
	UGameItem* DuplicateItem(UGameItem* Item) const;

	virtual void OnItemAdded(UGameItem* Item);
	virtual void OnItemRemoved(UGameItem* Item);

	/** Called when the underlying list has changed. */
	virtual void OnListChanged(FGameItemListEntry& Entry, int32 NewCount, int32 OldCount);

private:
	/** The replicated item list struct. */
	UPROPERTY(Replicated)
	FGameItemList ItemList;

public:
	/** Display debug info about this component. */
	virtual void DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) const;
};
