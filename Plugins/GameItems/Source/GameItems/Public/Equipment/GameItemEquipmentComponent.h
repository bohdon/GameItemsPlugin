// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameEquipmentComponent.h"
#include "GameItemEquipmentComponent.generated.h"

class UGameItem;
class UGameItemContainerComponent;


/**
 * Handles applying equipment that is granted from items.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEITEMS_API UGameItemEquipmentComponent : public UGameEquipmentComponent
{
	GENERATED_BODY()

public:
	UGameItemEquipmentComponent(const FObjectInitializer& ObjectInitializer);

	/** Return all equipment that was granted by an item. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Equipment")
	TArray<UGameEquipment*> FindAllEquipmentFromItem(UGameItem* Item) const;

	/** Add an item container as a source of items for providing equipment. */
	UFUNCTION(BlueprintCallable)
	void AddItemContainer(UGameItemContainerComponent* ItemContainer);

	/** Remove an item container as a source of items for providing equipment. */
	UFUNCTION(BlueprintCallable)
	void RemoveItemContainer(UGameItemContainerComponent* ItemContainer);

	UFUNCTION(BlueprintCallable)
	void ReapplyAllItemEquipment();

	virtual bool ShouldApplyEquipmentForItem(UGameItem* Item) const;

protected:
	/** The item containers to monitor for items with equipment. */
	UPROPERTY()
	TArray<TWeakObjectPtr<UGameItemContainerComponent>> ItemContainers;

	/** Map of equipment that was applied, indexed by the source item. */
	UPROPERTY()
	TMap<TObjectPtr<UGameItem>, TObjectPtr<UGameEquipment>> ItemEquipmentMap;

	UGameEquipment* ApplyEquipmentForItem(UGameItem* Item);

	void RemoveEquipmentForItem(UGameItem* Item);

	void OnItemAdded(UGameItem* Item);
	void OnItemRemoved(UGameItem* Item);
};
