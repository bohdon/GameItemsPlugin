// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GameItemDef.generated.h"


/**
 * Defines the maximum quantity of an item, and how it should handle stacking within a container.
 * For complex rules, see UGameItemContainerStockRules.
 */
USTRUCT(BlueprintType)
struct FGameItemSimpleStockRules
{
	GENERATED_BODY()

	FGameItemSimpleStockRules()
	{
	}

	/** Limit the maximum count of this item that can be in a container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem")
	bool bLimitCount = true;

	/** The maximum quantity allowed of this item in a container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition="bLimitCount", ClampMin = 1), Category = "GameItem")
	int32 MaxCount = 1;

	/** Can this item ever be stacked? Item containers should define the maximum count per stack if so. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem")
	bool bCanStack = false;
};


/**
 * Base class for a gameplay item definition. Designed to be subclassed in Blueprint,
 * then filled out using GameItemFragments.
 */
UCLASS(Blueprintable, Abstract)
class GAMEITEMS_API UGameItemDef : public UObject
{
	GENERATED_BODY()

public:
	/** The tags that this item has. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem")
	FGameplayTagContainer OwnedTags;

	/** The user-facing display name. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameItem")
	FText DisplayName;

	/** The maximum quantity and stacking rules for this item. Additional rules can be defined using UGameItemContainerStockRules. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameItem")
	FGameItemSimpleStockRules StockRules;

	/** The fragments that make up this item. Can be anything from UI data to gameplay functionality. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "GameItem")
	TArray<TObjectPtr<UGameItemFragment>> Fragments;

	/** Find and return a fragment of this item definition by class. */
	const UGameItemFragment* FindFragment(TSubclassOf<UGameItemFragment> FragmentClass) const;

	/** Find and return a fragment of this item definition by class. */
	template <class T>
	const T* FindFragment() const
	{
		return (T*)FindFragment(T::StaticClass());
	}

protected:
	UGameItemFragment* FindFragmentInternal(TSubclassOf<UGameItemFragment> FragmentClass) const;
};
