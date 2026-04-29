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

protected:
	/** The definition of the item. */
	UPROPERTY(Replicated, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	TSubclassOf<UGameItemDef> ItemDef;

	/** The quantity of this item in this instance (aka stack). */
	UPROPERTY(SaveGame, ReplicatedUsing = OnRep_Count, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	int32 Count;

	UFUNCTION()
	void OnRep_Count(int32 OldCount);

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
	virtual void CopyItemProperties(const UGameItem* Item);

	/** Return all containers that this item is in. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems")
	TArray<UGameItemContainer*> GetContainers() const;

	/** Return a debug string representation of this item instance. */
	UFUNCTION(BlueprintPure, Category = "GameItems")
	FString GetDebugString() const;

public:
	/** Is any network operation currently pending for this item? Used to block other predictive actions. */
	bool HasPendingNetChange() const;

	/** Clear any pending predicted network changes as part of acceptance. Does not perform any logic. */
	void AcceptPendingNetChange();

	/** Undo pending net changes, broadcasting events to reveal previous state. */
	void RejectPendingNetChange();

	const FGameItemsPredictionKey& GetPendingPredictionKey() const { return PendingPredictionKey; }

	/** Called when transferring a client-only item to the server, to indicate it's predicted this will be removed. */
	void MarkPendingRemove(UGameItemContainer* FromContainer, const FGameItemsPredictionKey PredictionKey);

	/**
	 * Called when client-predicting a move for this on server-owned containers.
	 * The actual move is irrelevant as it will be replicated from the server on when accepted.
	 */
	void MarkPendingMove(const FGameItemsPredictionKey PredictionKey);

	/** Called when transferring a client-only item to the server, to indicate it's predicted this will be removed. */
	void SetPendingCount(int32 NewCount, const FGameItemsPredictionKey PredictionKey);

	/** Return the container that this item will be removed from soon, while awaiting network confirmation. */
	UGameItemContainer* GetPendingRemoveContainer() const;

	/** Return the pending new count for this item, while awaiting network confirmation. */
	TOptional<int32> GetPendingCount() const;
	
protected:
	/** Clear all prediction / pending state variables (usually as part of accepting or rejecting the changes). */
	void ResetPredictionState();

public:
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FCountChangedDelegate, UGameItem* /*Item*/, int32 /*NewCount*/, int32 /*OldCount*/);
	DECLARE_MULTICAST_DELEGATE_FourParams(FTagStatChangedDelegate, UGameItem* /*Item*/, const FGameplayTag& /*Tag*/, int32 /*NewValue*/, int32 /*OldValue*/);
	DECLARE_MULTICAST_DELEGATE_FourParams(FSlottedDelegate, UGameItem* /*Item*/, const UGameItemContainer* /*Container*/, int32 /*NewSlot*/, int32 /*OldSlot*/);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FUnslottedDelegate, UGameItem* /*Item*/, const UGameItemContainer* /*Container*/, int32 /*OldSlot*/);

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
	TArray<TWeakObjectPtr<UGameItemContainer>> Containers;

	FGameItemsPredictionKey PendingPredictionKey;
	bool bIsPendingRemove;
	TWeakObjectPtr<UGameItemContainer> PendingRemoveContainer;
	TOptional<int32> PendingCount;

	friend UGameItemContainer;
};
