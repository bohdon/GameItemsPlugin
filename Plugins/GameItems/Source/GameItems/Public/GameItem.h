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
	UPROPERTY(Replicated, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	TSubclassOf<UGameItemDef> ItemDef;

	/** The quantity of this item in this instance (aka stack). */
	UPROPERTY(SaveGame, Replicated, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	int32 Count;

	/** Tags representing various stats about this item, such as level, use count, remaining ammo, etc. */
	UPROPERTY(SaveGame, Replicated)
	FGameItemTagStackContainer TagStats;

public:
	FORCEINLINE TSubclassOf<UGameItemDef> GetItemDef() const { return ItemDef; }

	/** Return the class default object for this item's definition. */
	UFUNCTION(BlueprintPure, Category = "GameItems")
	const UGameItemDef* GetItemDefCDO() const;

	void SetItemDef(TSubclassOf<UGameItemDef> NewItemDef);

	/** Return the owned tags of the item definition. */
	UFUNCTION(BlueprintPure, Category = "GameItems")
	const FGameplayTagContainer& GetOwnedTags() const;

	FORCEINLINE int32 GetCount() const { return Count; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	void SetCount(int32 NewCount);

	/** Return true if the item has a stat. */
	UFUNCTION(BlueprintPure, Category = "GameItems", meta = (GameplayTagFilter = "GameItemStatTagsCategory"))
	bool HasTagStat(FGameplayTag Tag) const;

	/** Set the value of a stat. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems", meta = (GameplayTagFilter = "GameItemStatTagsCategory"))
	void SetTagStat(FGameplayTag Tag, int32 NewCount);

	/** Return the value of a stat, or 0 if it doesn't exist. */
	UFUNCTION(BlueprintPure, Category = "GameItems", meta = (GameplayTagFilter = "GameItemStatTagsCategory"))
	int32 GetTagStat(FGameplayTag Tag) const;

	/** Increase the value of a stat. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems", meta = (GameplayTagFilter = "GameItemStatTagsCategory"))
	void AddTagStat(FGameplayTag Tag, int32 DeltaValue);

	/** Decrease the value of a stat. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems", meta = (GameplayTagFilter = "GameItemStatTagsCategory"))
	void RemoveTagStat(FGameplayTag Tag, int32 DeltaValue);

	/** Return a map of all tag stats and their values. */
	UFUNCTION(BlueprintPure, Category = "GameItems")
	const TMap<FGameplayTag, int32>& GetAllTagStats() const;

	/** Return the tag stats container. */
	const FGameItemTagStackContainer& GetTagStatsContainer() const { return TagStats; }

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
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	TArray<UGameItemContainer*> GetContainers() const;

	/** Return a debug string representation of this item instance. */
	UFUNCTION(BlueprintPure, Category = "GameItems")
	FString GetDebugString() const;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FCountChangedDelegate, int32 /*NewCount*/, int32 /*OldCount*/);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FTagStatChangedDelegate, const FGameplayTag& /*Tag*/, int32 /*NewValue*/, int32 /*OldValue*/);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FSlottedDelegate, const UGameItemContainer* /*Container*/, int32 /*NewSlot*/, int32 /*OldSlot*/);
	DECLARE_MULTICAST_DELEGATE_TwoParams(FUnslottedDelegate, const UGameItemContainer* /*Container*/, int32 /*OldSlot*/);

	/** Called when the quantity of this item stack has changed. */
	FCountChangedDelegate OnCountChangedEvent;

	/** Called when a tag stat of this item has changed. */
	FTagStatChangedDelegate OnTagStatChangedEvent;

	/** Called when this item is added to any container. */
	FSlottedDelegate OnSlottedEvent;

	/** Called when this item is removed from any container. */
	FUnslottedDelegate OnUnslottedEvent;

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** All containers that this item is in. */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UGameItemContainer>> Containers;

	friend UGameItemContainer;
};
