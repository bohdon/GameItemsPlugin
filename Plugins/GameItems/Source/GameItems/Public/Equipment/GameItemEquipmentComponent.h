// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameEquipmentComponent.h"
#include "WorldConditionQuery.h"
#include "GameItemEquipmentComponent.generated.h"

class UGameItem;
class UGameItemContainer;
class UGameItemContainerComponent;
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
 * Monitors items in a UGameItemContainerComponent/collection (rather than individual containers).
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEITEMS_API UGameItemEquipmentComponent : public UGameEquipmentComponent
{
	GENERATED_BODY()

public:
	UGameItemEquipmentComponent(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;

	/** Monitor all items in a component, and apply any equipment from existing items whose conditions are met. */
	UFUNCTION(BlueprintCallable, Category = "GameEquipment")
	void RegisterItemContainerComponent(UGameItemContainerComponent* ItemContainerComponent);

	/** Stop monitoring an item container component, and remove any equipment applied by its items. */
	UFUNCTION(BlueprintCallable, Category = "GameEquipment")
	void UnregisterItemContainerComponent(UGameItemContainerComponent* ItemContainerComponent);

	/** Return all currently registered container components. */
	const TArray<TWeakObjectPtr<UGameItemContainerComponent>>& GetRegisteredContainerComponents() const { return RegisteredContainerComponents; }

	/**
	 * Return the equipment fragment to use for an item.
	 * Can be overridden to find a specific fragment if there are potentially multiple on one item.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Equipment")
	const UGameItemFragment_Equipment* GetItemEquipmentFragment(UGameItem* Item) const;

	/** Return all equipment that was granted by an item. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Equipment")
	UGameEquipment* FindAllEquipmentFromItem(UGameItem* Item) const;

protected:
	/**
	 * Return true if this component should handle applying equipment for an item.
	 * Useful for filtering items in case multiple equipment components should handle different types of items.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Equipment")
	bool ShouldHandleItemEquipment(UGameItem* Item) const;

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

public:
	/** Automatically register any UGameItemContainerComponent found on the owner during InitializeComponent. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bAutoFindContainerComponent = true;

protected:
	/** The item containers to monitor for items with equipment. */
	TArray<TWeakObjectPtr<UGameItemContainerComponent>> RegisteredContainerComponents;

	/** Map of condition states for each item with equipment in the target containers. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UGameItem>, FGameItemEquipmentConditionState> ItemConditionStates;

	/** Map of equipment definitions that were applied by source item, for removal. */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UGameItem>, TSubclassOf<UGameEquipmentDef>> ItemEquipmentDefs;
};
