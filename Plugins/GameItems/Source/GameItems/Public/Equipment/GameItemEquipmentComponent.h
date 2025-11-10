// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameEquipmentComponent.h"
#include "GameplayTagContainer.h"
#include "WorldConditionQuery.h"
#include "GameItemEquipmentComponent.generated.h"

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

	/** Container Ids to find and add as sources during initialization. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (GameplayTagFilter = "GameItemContainerIdTagsCategory"))
	FGameplayTagContainer StartupContainerIds;

	/**
	 * Automatically find and add startup containers on BeginPlay.
	 * Disable this if the item containers won't be ready until later, and call AddStartupItemContainers when they are.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (GameplayTagFilter = "GameItemContainerIdTagsCategory"))
	bool bAutoAddStartupContainers = true;

	/** Return all equipment that was granted by an item. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Equipment")
	TArray<UGameEquipment*> FindAllEquipmentFromItem(UGameItem* Item) const;

	/** Add an item container as a source of items for providing equipment. */
	UFUNCTION(BlueprintCallable)
	void AddItemContainer(UGameItemContainer* ItemContainer);

	/** Remove an item container as a source of items for providing equipment. */
	UFUNCTION(BlueprintCallable)
	void RemoveItemContainer(UGameItemContainer* ItemContainer);

	UFUNCTION(BlueprintCallable)
	void ReapplyAllItemEquipment();

	/** Find and add all startup item containers by id. */
	UFUNCTION(BlueprintCallable)
	void AddStartupItemContainers();

	/** Return the equipment fragment for an item, or null if it has one or the fragment is invalid. */
	virtual const UGameItemFragment_Equipment* GetItemEquipmentFragment(UGameItem* Item) const;

	virtual void BeginPlay() override;

protected:
	/** The item containers to monitor for items with equipment. */
	TArray<TWeakObjectPtr<UGameItemContainer>> ItemContainers;

	/** Map of condition states for each item with equipment in the target containers. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UGameItem>, FGameItemEquipmentConditionState> ItemConditionStates;

	/** Map of equipment that was applied, indexed by the source item. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UGameItem>, TObjectPtr<UGameEquipment>> ItemEquipmentMap;

	/** Activate the equipment conditions for an item, and apply the equipment if met. */
	void ActivateItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag);

	/** Check and re-apply the equipment for an item if the conditions are met. */
	void CheckItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag);

	/** Remove equipment and deactivate the conditions for an item. */
	void DeactivateItemEquipmentCondition(UGameItem* Item, const UGameItemFragment_Equipment* EquipFrag);

	/** Provide context references for an item equipment condition. */
	void SetupConditionContextData(FWorldConditionContextData& ContextData, const UGameItem* Item) const;

	UGameEquipment* ApplyEquipmentForItem(UGameItem* Item);
	void RemoveEquipmentForItem(UGameItem* Item);

	void OnItemAdded(UGameItem* Item);
	void OnItemRemoved(UGameItem* Item);
	void OnExistingItemSlotted(const UGameItemContainer* Container, int32 NewSlot, int32 OldSlot, UGameItem* Item);
	void OnExistingItemUnslotted(const UGameItemContainer* Container, int32 OldSlot, UGameItem* Item);
};
