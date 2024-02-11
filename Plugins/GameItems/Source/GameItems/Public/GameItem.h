// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "GameItem.generated.h"

class UGameItemDef;
class UGameItemContainer;


/**
 * An instance of a game item.
 */
UCLASS(BlueprintType)
class GAMEITEMS_API UGameItem : public UObject
{
	GENERATED_BODY()

public:
	UGameItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
	/** The definition of the item. */
	UPROPERTY(Replicated, BlueprintReadOnly, DisplayName = "ItemDefClass", Meta = (AllowPrivateAccess))
	TSubclassOf<UGameItemDef> ItemDef;

	/** The quantity of this item in this instance (aka stack). */
	UPROPERTY(Replicated, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	int32 Count;

	/** Tags representing various stats about this item, such as level, use count, remaining ammo, etc. */
	UPROPERTY(Replicated)
	FGameItemTagStackContainer TagStats;

public:
	TSubclassOf<UGameItemDef> GetItemDef() const { return ItemDef; }

	/** Return the class default object for this item's definition. */
	UFUNCTION(BlueprintPure, DisplayName = "GetItemDef")
	const UGameItemDef* GetItemDefCDO() const;

	void SetItemDef(TSubclassOf<UGameItemDef> NewItemDef);

	/** Return the owned tags of the item definition. */
	UFUNCTION(BlueprintPure)
	const FGameplayTagContainer& GetOwnedTags() const;

	int32 GetCount() const { return Count; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItem")
	void SetCount(int32 NewCount);

	/** Increase the value of a stat tag. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItem")
	void AddTagStat(FGameplayTag Tag, int32 DeltaValue);

	/** Decrease the value of a stat tag. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItem")
	void RemoveTagStat(FGameplayTag Tag, int32 DeltaValue);

	/** Return the value of a stat. */
	UFUNCTION(BlueprintPure, Category = "GameItem")
	int32 GetTagStat(FGameplayTag Tag) const;

	/**
	 * Return true if another item matches this item. Matching items must always have the same definition,
	 * but additional requirements can be defined to prevent items from stacking where undesired.
	 * For example when leveling up items using tag stats, this can prevent lv1 items from stacking with lv2 items of the same definition.
	 */
	bool IsMatching(const UGameItem* Item) const;

	/**
	 * Copy all properties from another item, such as count and tag stats.
	 * Does not broadcast any events.
	 */
	void CopyItemProperties(const UGameItem* Item);

	/** Return all containers that this item is in. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	TArray<UGameItemContainer*> GetContainers() const;

	/** Return a debug string representation of this item instance. */
	UFUNCTION(BlueprintPure, Category = "GameItem")
	FString ToDebugString() const;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FCountChangedDelegate, int32 /*NewCount*/, int32 /*OldCount*/);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FTagStatChangedDelegate, const FGameplayTag& /*Tag*/, int32 /*NewValue*/, int32 /*OldValue*/);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FSlottedDelegate, const UGameItemContainer* /*Container*/, int32 /*NewSlot*/, int32 /*OldSlot*/);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FUnslottedDelegate, const UGameItemContainer* /*Container*/, int32 /*OldSlot*/);

	/** Called when the quantity of this item stack has changed. */
	FCountChangedDelegate OnCountChangedEvent;

	/** Called when a tag stat of this item has changed. */
	FTagStatChangedDelegate OnTagStatChangedEvent;

	/** Called when this item is added or moved in any container. */
	FSlottedDelegate OnSlottedEvent;

	/** Called when this item is removed from any container. */
	FUnslottedDelegate OnUnslottedEvent;

	virtual bool IsSupportedForNetworking() const override { return true; }

protected:
	/** All containers that this item is in. */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UGameItemContainer>> Containers;

	friend UGameItemContainer;
};
