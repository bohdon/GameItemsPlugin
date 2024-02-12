// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItem.h"
#include "GameItemTypes.h"
#include "UObject/Object.h"
#include "GameItemContainer.generated.h"

class UGameItem;
class UGameItemContainerDef;
class UGameItemContainerLink;
class UGameItemContainerRule;
class UGameItemDef;
class UGameItemSet;


/**
 * Represents a plan for adding an item to a container.
 * Can be used to verify the results before adding the item.
 */
USTRUCT(BlueprintType)
struct FGameItemContainerAddPlan
{
	GENERATED_BODY()

	/** The slots where the item will be added. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> TargetSlots;

	/** The amounts to add for each slot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int32> SlotDeltaCounts;

	/** The total quantity that will be added across all slots. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DeltaCount = 0;

	/** The remainder quantity that cannot be added, if unable to add the full amount. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RemainderCount = 0;

	/** Will the full quantity of the item be added? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bWillAddFullAmount = false;

	/** Will the item be split when added? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bWillSplit = false;

	void AddCountToSlot(int32 Slot, int32 Count);

	/** Update total counts and statuses for the plan. */
	void UpdateDerivedValues(int32 ItemCount);
};


/**
 * Object that contains one or more game item instances,
 * like an inventory, treasure chest, or just a simple item pickup.
 */
UCLASS(BlueprintType, Blueprintable)
class GAMEITEMS_API UGameItemContainer : public UObject
{
	GENERATED_BODY()

public:
	UGameItemContainer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** The unique tag identifying this container amongst others. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItemContainer")
	FGameplayTag ContainerId;

	/** The user-facing display name of this container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItemContainer")
	FText DisplayName;

	/** Set the definition for this container. Cannot be changed once set. */
	UFUNCTION(BlueprintCallable)
	void SetContainerDef(TSubclassOf<UGameItemContainerDef> NewContainerDef);

	/** Return the CDO of the container definition. */
	UFUNCTION(BlueprintPure, DisplayName = "GetContainerDef")
	FORCEINLINE const UGameItemContainerDef* GetContainerDefCDO() const;

	/** Get all tags that this container has. */
	UFUNCTION(BlueprintCallable, Category = "GameItemContainer")
	FGameplayTagContainer GetOwnedTags() const;

	/** Return true if a new item could be added to this container. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Meta = (DeprecatedFunction), Category = "GameItemContainer")
	bool CanAddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/**
	 * Add one or more of a new item to this container.
	 * @return The newly created item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Meta = (DeprecatedFunction), Category = "GameItemContainer")
	UGameItem* AddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/** Check if an item can be fully added to a container and whether it will be split when added. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	FGameItemContainerAddPlan CheckAddItem(UGameItem* Item, int32 TargetSlot = -1) const;

	/**
	 * Add an item to this container. This does not remove the item from any existing containers.
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
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	UGameItem* GetItemAt(int32 Slot) const;

	/** Return the item in teh first slot of the container. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	UGameItem* GetFirstItem() const;

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
	int32 GetItemSlot(const UGameItem* Item) const;

	/** Return true if an item exists in the container. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	bool Contains(const UGameItem* Item) const;

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

	/**
	 * Return the number of slots in the container.
	 * If slot count is unlimited, return the number of items.
	 */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	int32 GetNumSlots() const;

	/** Return the number of empty slots available in this container, or -1 if unlimited. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	int32 GetNumEmptySlots() const;

	/** Return the next available empty slot in this container, or -1 if no slots are available. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	int32 GetNextEmptySlot() const;

	/** Return true if a slot is a valid index into this container. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	bool IsValidSlot(int32 Slot) const;

	/** Return true if a slot in this container is empty and available. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	bool IsSlotEmpty(int32 Slot) const;

	/** Return true if the container is ever allowed to contain an item. */
	UFUNCTION(BlueprintPure, Category = "GameItemContainer")
	bool CanContainItem(const UGameItem* Item) const;

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

	/** Return all rules applied to this container. */
	const TArray<UGameItemContainerRule*>& GetRules() const { return Rules; }

	/** Add a container rule. */
	UGameItemContainerRule* AddRule(TSubclassOf<UGameItemContainerRule> RuleClass);

	/** Add a container rule. */
	template <class T>
	T* AddRule(TSubclassOf<UGameItemContainerRule> RuleClass)
	{
		static_assert(TIsDerivedFrom<T, UGameItemContainerRule>::IsDerived, "T must be a UGameItemContainerRule type");
		return Cast<T>(AddRule(RuleClass));
	}

	/**
	 * Remove a container rule by class.
	 * @return The number of rules that were removed.
	 */
	int32 RemoveRule(TSubclassOf<UGameItemContainerRule> RuleClass);

	/** Return the owning actor of this container. */
	virtual AActor* GetOwner() const;

	virtual UWorld* GetWorld() const override;

	DECLARE_MULTICAST_DELEGATE_OneParam(FItemAddOrRemoveDelegate, UGameItem* /*Item*/);
	DECLARE_MULTICAST_DELEGATE_OneParam(FItemSlotChangedDelegate, int32 /*Slot*/);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FItemSlotsChangedDelegate, int32 /*StartSlot*/, int32 /*EndSlot*/);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FNumSlotsChangedDelegate, int32 /*NewNumSlots*/, int32 /*OldNumSlots*/);

	/** Called when a new item is added. */
	FItemAddOrRemoveDelegate OnItemAddedEvent;

	/** Called when an item is removed. */
	FItemAddOrRemoveDelegate OnItemRemovedEvent;

	/** Called the item in a slot is changed. */
	FItemSlotChangedDelegate OnItemSlotChangedEvent;

	/** Called when a range of item slots have changed. */
	FItemSlotsChangedDelegate OnItemSlotsChangedEvent;

	/** Called when the total number of slots has changed. */
	FNumSlotsChangedDelegate OnNumSlotsChangedEvent;

protected:
	/** The settings for this container. */
	UPROPERTY(Transient, BlueprintReadOnly, DisplayName = "ContainerDefClass", Meta = (AllowPrivateAccess = true), Category = "GameItemContainer")
	TSubclassOf<UGameItemContainerDef> ContainerDef;

	/** The active rules applied to this container. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "GameItemContainer")
	TArray<TObjectPtr<UGameItemContainerRule>> Rules;

	/** Have the default items already been added to this container? */
	bool bHasDefaultItems;

	/** Used to track slot changes and broadcast after several operations are completed. */
	struct FScopedSlotChanges
	{
		FScopedSlotChanges(UGameItemContainer* InContainer)
			: Container(InContainer)
		{
			check(Container);
			Container->BeginSlotChanges();
		}

		~FScopedSlotChanges()
		{
			if (Container)
			{
				Container->EndSlotChanges();
			}
		}

		UGameItemContainer* Container;
	};

	/** Number of open change operations. */
	int32 ActiveChangeOperations;

	/** Number of slots in the container before any change operations. */
	int32 NumSlotsPreChange;

	/** Set of slots that were changed during change operations. */
	TArray<int32> ChangedSlots;

	/** Create and return a new item instance using the GameItemSubsystem. */
	UGameItem* CreateItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count) const;

	/** Duplicate and return a new item instance using the GameItemSubsystem. */
	UGameItem* DuplicateItem(UGameItem* Item) const;

	/**
	 * Return a plan representing how an item will be added to this container,
	 * including exactly which slots and quantities should be added.
	 * Can be used to check for item loss or split before adding.
	 */
	FGameItemContainerAddPlan GetAddItemPlan(UGameItem* Item, int32 TargetSlot = -1, bool bWarn = true) const;

	virtual void OnItemAdded(UGameItem* Item, int32 Slot);
	virtual void OnItemRemoved(UGameItem* Item, int32 Slot);

	/** Start a slot change operation, gathering slot changes to broadcast later. */
	void BeginSlotChanges();

	/** End a slot change operation. Once all operations are finished, the changes will be broadcast. */
	void EndSlotChanges();

	void OnSlotChanged(int32 Slot);
	void OnSlotsChanged(const TArray<int32>& Slots);
	void OnSlotRangeChanged(int32 StartSlot, int32 EndSlot);

	void BroadcastSlotChanges();

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
