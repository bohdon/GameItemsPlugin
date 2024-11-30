// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemDropContext.h"
#include "GameItemSetEntrySelector.h"
#include "GameItemTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "Engine/DataTable.h"
#include "Templates/SubclassOf.h"
#include "GameItemDropContent.generated.h"

class UDataTable;
class UGameItemDef;
class UGameItemSet;
class UGameItemSetEntrySelector;
class UGameItemSetEntrySelector_Random;


/**
 * Defines the items and quantities to spawn
 * for a selected entry in a game item drop.
 */
USTRUCT(BlueprintType)
struct FGameItemDropContent
{
	GENERATED_BODY()

	/* The name or description of the content for designers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayPriority = -10))
	FText DesignerNotes = INVTEXT("");

	virtual ~FGameItemDropContent() = default;

	/** Perform a random check to see if any content should be given. */
	virtual bool ShouldGiveContent() const;

	/** Perform a random probability test, then select and return items for this content if passed. */
	virtual void CheckAndSelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const;

	/** Select and return items for this content. */
	virtual void SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const;

	/** Called when the owning data table row has changed. */
	virtual void OnDataChanged();
};


/** Base class for content that has a chance to give anything, or otherwise yield nothing. */
USTRUCT(BlueprintType)
struct FGameItemDropChancedContent : public FGameItemDropContent
{
	GENERATED_BODY()

	/* The chance that any items will be given. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "0", ClampMax = "1", DisplayPriority = -1))
	float Chance = 1.f;

	virtual bool ShouldGiveContent() const override;
};


/** Combines multiple drop contents together. */
USTRUCT(BlueprintType, DisplayName = "Combine")
struct FGameItemDropContent_Combine : public FGameItemDropChancedContent
{
	GENERATED_BODY()

	/** The list of content to combine. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExcludeBaseStruct, ShowTreeView))
	TArray<TInstancedStruct<FGameItemDropContent>> Contents;

	virtual void SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const override;
};


/** Defines FGameItemDropContent with an associated probability for selection. */
USTRUCT(BlueprintType)
struct FWeightedGameItemDropContent
{
	GENERATED_BODY()

	/* The relative probability that this content will be selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "0"))
	float Probability = 1.f;

	/** The content. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExcludeBaseStruct, ShowTreeView))
	TInstancedStruct<FGameItemDropContent> Content;
};


/** Selects a random single content. */
USTRUCT(BlueprintType, DisplayName = "Select")
struct FGameItemDropContent_Select : public FGameItemDropChancedContent
{
	GENERATED_BODY()

	/** The contents to choose from. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeightedGameItemDropContent> WeightedContents;

	virtual void SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const override;
};


/** An explicit item and count. */
USTRUCT(BlueprintType, DisplayName = "Item")
struct FGameItemDropContent_Item : public FGameItemDropChancedContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowAbstract = false))
	TSoftClassPtr<UGameItemDef> ItemDef = nullptr;

	virtual void SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const override;
};


/** Provides one or more items from an item set. */
USTRUCT(BlueprintType, DisplayName = "Item Set")
struct FGameItemDropContent_ItemSet : public FGameItemDropChancedContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UGameItemSet> ItemSet = nullptr;

	/** The class that defines which item or items to provide from the set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameItemSetEntrySelector> SelectorClass = UGameItemSetEntrySelector_Random::StaticClass();

	/**
	 * Contextual parameters that are passed along to affect drop content.
	 * These can be anything from currency/value multipliers to item filters and rarity modifiers.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TInstancedStruct<FGameItemDropParams> Params;

	virtual void SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const override;
};


/** Selects entries from another drop table as the content for this drop. */
USTRUCT(BlueprintType, DisplayName = "Drop Table Entry")
struct FGameItemDropContent_DropTableEntry : public FGameItemDropChancedContent
{
	GENERATED_BODY()

	/**
	 * Contextual parameters that are passed along to affect drop content.
	 * These can be anything from currency/value multipliers to item filters and rarity modifiers.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TInstancedStruct<FGameItemDropParams> Params;

	/** The drop table from which to select entries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType = "/Script/GameItems.GameItemDropTableRow"))
	FDataTableRowHandle DropTableRow;

	virtual void SelectItems(const FGameItemDropContext& Context, TArray<FGameItemDefStack>& OutItems) const override;
};
