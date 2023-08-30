// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "Components/ActorComponent.h"
#include "GameItemContainerComponent.generated.h"

class UGameItem;
class UGameItemDef;


/**
 * Component that contains one or more game item instances,
 * like an inventory, treasure chest, or just a simple item pickup.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEITEMS_API UGameItemContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameItemContainerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Return true if a new item can be added to this container. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItemContainer")
	bool CanAddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/**
	 * Add one or more of a new item to this container.
	 * @return The newly created item.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItemContainer")
	UGameItem* AddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count = 1);

	/** Add an item to this container. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItemContainer")
	void AddItem(UGameItem* Item);

	/** Remove an item from this container. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItemContainer")
	void RemoveItem(UGameItem* Item);

	/** Return all items in the container. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	TArray<UGameItem*> GetAllItems() const;

	/** Return the first stack of an item by definition. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = false, Category = "GameItemContainer")
	UGameItem* FindFirstItemByDef(TSubclassOf<UGameItemDef> ItemDef) const;

	/** Return the total number of an item in this container by definition. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItemContainer")
	int32 GetTotalItemCountByDef(TSubclassOf<UGameItemDef> ItemDef) const;

	// UObject interface
	virtual void ReadyForReplication() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	/** Create and return a new item instance using the GameItemSubsystem. */
	UGameItem* CreateItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count) const;

	virtual void OnItemAdded(UGameItem* Item);
	virtual void OnItemRemoved(UGameItem* Item);

	/** Called when the underlying list has changed. */
	virtual void OnListChanged(FGameItemListEntry& Entry, int32 NewCount, int32 OldCount);

private:
	/** The replicated item list struct. */
	UPROPERTY(Replicated)
	FGameItemList ItemList;
};
