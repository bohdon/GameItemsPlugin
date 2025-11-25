// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemFragment.h"
#include "GameItemTypes.h"
#include "GameplayTagContainer.h"
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
	UGameItemDef(const FObjectInitializer& ObjectInitializer);

	/**
	 * The game item class to instances for this definition.
	 * This allows extending transient or saved state per item instance.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem", NoClear, AdvancedDisplay)
	TSubclassOf<UGameItem> ItemClass;

	/** The tags that this item has. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem", meta = (GameplayTagFilter="GameItemTagsCategory"))
	FGameplayTagContainer OwnedTags;

	/** The user-facing display name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem")
	FText DisplayName;

	/** The maximum quantity of this item in a collection. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem")
	FGameItemCountLimit CollectionLimit;

	/** The maximum quantity of this item in a single container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem")
	FGameItemCountLimit ContainerLimit;

	/** The maximum quantity of this item in a single stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem")
	FGameItemCountLimit StackLimit;

	/** The fragments that make up this item. Can be anything from UI data to gameplay functionality. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "GameItem")
	TArray<TObjectPtr<UGameItemFragment>> Fragments;

	/** Find and return a fragment of this item definition by class. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems", Meta = (DeterminesOutputType = "FragmentClass"))
	const UGameItemFragment* FindFragment(TSubclassOf<UGameItemFragment> FragmentClass) const;

	/** Find and return a fragment of this item definition by class. */
	template <class T>
	const T* FindFragment() const
	{
		static_assert(TIsDerivedFrom<T, UGameItemFragment>::IsDerived, TEXT("T must be a UGameItemFragment type"));
		return (T*)FindFragment(T::StaticClass());
	}

#if WITH_EDITOR
	/** Return the editor icon for this item. */
	virtual TOptional<struct FSlateBrush> GetEditorIcon() const;
#endif

protected:
	UGameItemFragment* FindFragmentInternal(TSubclassOf<UGameItemFragment> FragmentClass) const;
};
