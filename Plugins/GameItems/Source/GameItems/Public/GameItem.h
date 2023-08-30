// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "UObject/Object.h"
#include "GameItem.generated.h"

class UGameItemDef;


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
	UPROPERTY(Replicated, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	int32 Count;

	/** Tags representing various stats about this item, such as level, use count, remaining ammo, etc. */
	UPROPERTY(Replicated)
	FGameItemTagStackContainer TagStats;

public:
	TSubclassOf<UGameItemDef> GetItemDef() const { return ItemDef; }

	void SetItemDef(TSubclassOf<UGameItemDef> NewItemDef);

	int32 GetCount() const { return Count; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItem")
	void SetCount(int32 NewCount);

	/** Increase the value of a stat tag by delta count. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItem")
	void AddTagStat(FGameplayTag Tag, int32 DeltaCount);

	/** Decrease the value of a stat tag by delta count. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItem")
	void RemoveTagStat(FGameplayTag Tag, int32 DeltaCount);

	/** Return the value of a stat. */
	UFUNCTION(BlueprintPure, Category = "GameItem")
	int32 GetTagStat(FGameplayTag Tag) const;

	/** Return a debug string representation of this item instance. */
	UFUNCTION(BlueprintPure, Category = "GameItem")
	FString ToDebugString() const;

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FTagStatChangedDelegate,
	                                       const FGameplayTag& /* Tag */,
	                                       int32 /* NewValue */,
	                                       int32 /* OldValue */);

	/** Called when a tag stat of this item has changed. */
	FTagStatChangedDelegate OnTagStatChangedEvent;


	virtual bool IsSupportedForNetworking() const override { return true; }
};
