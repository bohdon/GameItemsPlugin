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
	/** Tags representing various stats about this item, such as level, use count, remaining ammo, etc. */
	UPROPERTY(Replicated)
	FGameItemTagStackContainer TagStats;

	/** The definition of the item. */
	UPROPERTY(Replicated)
	TSubclassOf<UGameItemDef> ItemDef;

public:
	/** Increase the value of a stat tag by delta count. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItem")
	void AddTagStat(FGameplayTag Tag, int32 DeltaCount);

	/** Decrease the value of a stat tag by delta count. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItem")
	void RemoveTagStat(FGameplayTag Tag, int32 DeltaCount);

	/** Return the value of a stat. */
	UFUNCTION(BlueprintPure, Category = "GameItem")
	int32 GetTagStat(FGameplayTag Tag) const;


	DECLARE_MULTICAST_DELEGATE_ThreeParams(FTagStatChangedDelegate,
	                                       const FGameplayTag& /* Tag */,
	                                       int32 /* NewValue */,
	                                       int32 /* OldValue */);

	/** Called when a tag stat of this item has changed. */
	FTagStatChangedDelegate OnTagStatChangedEvent;


	virtual bool IsSupportedForNetworking() const override { return true; }
};
