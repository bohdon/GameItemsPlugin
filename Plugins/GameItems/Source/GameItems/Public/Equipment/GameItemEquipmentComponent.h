// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameEquipmentComponent.h"
#include "GameplayTagContainer.h"
#include "WorldConditionQuery.h"
#include "GameItemEquipmentComponent.generated.h"

class UGameItemContainerComponent;
class UGameItem;
class UGameItemContainer;
class UGameItemFragment_Equipment;


/**
 * Runtime state managing the condition
 * for applying equipment for an item.
 */
USTRUCT()
struct FGameItemEquipmentConditionState
{
	GENERATED_BODY()

	/** The item instance with the equipment condition. */
	UPROPERTY(Transient)
	TObjectPtr<UGameItem> Item;

	/** The conditions state. */
	UPROPERTY(Transient)
	FWorldConditionQueryState State;
};


/**
 * A UGameEquipmentComponent that handles applying equipment granted by items.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEITEMS_API UGameItemEquipmentComponent : public UGameEquipmentComponent
{
	GENERATED_BODY()

public:
	UGameItemEquipmentComponent(const FObjectInitializer& ObjectInitializer);

	/**
	 * Query used to filter game item containers when finding or monitoring containers.
	 * If empty, all containers are included.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (GameplayTagFilter = "GameItemContainerIdTagsCategory"))
	FGameplayTagQuery ContainerQuery;

	/**
	 * Only add parent containers (that actually store items) when finding or monitoring containers.
	 * Item slotted events allow monitoring changes that are relevant to equipment conditions, so it's often
	 * desired to monitor only parent containers in order to detect the presence of new items.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (GameplayTagFilter = "GameItemContainerIdTagsCategory"))
	bool bIgnoreChildContainers = true;

	/**
	 * Automatically call FindAllItemContainers on BeginPlay, using the default container query,
	 * and ignoring child containers. It's common to disable this and add containers once the character
	 * is initialized and ready for equipment.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (GameplayTagFilter = "GameItemContainerIdTagsCategory"))
	bool bAutoFindContainers = true;

	/** Return all equipment that was granted by an item. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Equipment")
	TArray<UGameEquipment*> FindAllEquipmentFromItem(UGameItem* Item) const;

	/** Add an item container as a source of items for providing equipment. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameEquipment")
	void AddItemContainer(UGameItemContainer* ItemContainer);

	/** Remove an item container as a source of items for providing equipment. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameEquipment")
	void RemoveItemContainer(UGameItemContainer* ItemContainer);

	/** Return all currently registered containers. */
	const TArray<TWeakObjectPtr<UGameItemContainer>>& GetRegisteredContainers() const { return RegisteredContainers; }

	/**
	 * Find and add all item containers matching the query, and optionally
	 * listen for containers added or removed on the item component.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameEquipment")
	void FindItemContainers(bool bMonitorNewContainers = true);

	/** Monitor containers being added/removed from an item component, and register any that match the container query. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameEquipment")
	void MonitorItemContainersFromComponent(UGameItemContainerComponent* ItemContainerComponent);

	/** Return the equipment fragment for an item, or null if it has one or the fragment is invalid. */
	virtual const UGameItemFragment_Equipment* GetItemEquipmentFragment(UGameItem* Item) const;

	virtual void BeginPlay() override;
	virtual void UninitializeComponent() override;

protected:
	/** The item containers to monitor for items with equipment. */
	TArray<TWeakObjectPtr<UGameItemContainer>> RegisteredContainers;

	/** Map of condition states for each item with equipment in the target containers. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UGameItem>, FGameItemEquipmentConditionState> ItemConditionStates;

	/** Map of equipment that was applied, indexed by the source item. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UGameItem>, TObjectPtr<UGameEquipment>> EquipmentMap;

	virtual bool ShouldIncludeContainer(UGameItemContainer* Container) const;

	/** Activate the equipment conditions for an item, and apply the equipment if met. */
	void ActivateItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag);

	/** Check and re-apply the equipment for an item if the conditions are met. */
	void CheckItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag);

	/** Remove equipment and deactivate the conditions for an item. */
	void DeactivateItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag);

	/** Provide context references for an item equipment condition. */
	void SetupConditionContextData(FWorldConditionContextData& ContextData, const UGameItem* Item) const;

	void ApplyEquipmentForItem(UGameItem* Item);
	void RemoveEquipmentForItem(UGameItem* Item);

	void OnItemAdded(UGameItem* Item);
	void OnItemRemoved(UGameItem* Item);
	void OnExistingItemSlotted(const UGameItemContainer* Container, int32 NewSlot, int32 OldSlot, UGameItem* Item);
	void OnExistingItemUnslotted(const UGameItemContainer* Container, int32 OldSlot, UGameItem* Item);
};
