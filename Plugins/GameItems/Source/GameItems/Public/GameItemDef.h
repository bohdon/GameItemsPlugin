// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "GameItemTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GameItemDef.generated.h"


/**
 * Base class for a gameplay item definition.
 * Designed to be subclassed in Blueprint for each item type and filled out using GameItemFragments.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const)
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

	/** The maximum quantity and stacking rules for this item. Additional rules can be defined using UGameItemContainerStockRule. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameItem")
	FGameItemStockRules StockRules;

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
