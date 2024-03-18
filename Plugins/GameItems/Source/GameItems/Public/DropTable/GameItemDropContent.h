// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemSetEntrySelector.h"
#include "GameItemTypes.h"
#include "Engine/DataTable.h"
#include "Templates/SubclassOf.h"
#include "GameItemDropContent.generated.h"

class UDataTable;
class UGameItemDef;
class UGameItemSet;
class UGameItemSetEntrySelector;
class UGameItemSetEntrySelector_Random;


/**
 * Defines a probability, optionally using inverse odds
 * to specify extremely rare chances with high precision.
 */
USTRUCT(BlueprintType)
struct FGameItemDropProbability
{
	GENERATED_BODY()

	/** The probability that any items will be given. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "!bUseOdds", ClampMin = "0", ClampMax = "1"))
	float Probability = 1.f;

	/** Use inverse odds to define the probability, e.g. 1 / Odds, to support extremely rare probabilities. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (InlineEditConditionToggle))
	bool bUseOdds = false;

	/**
	 * The odds that no items will be given, e.g.
	 *		1 = 100% (1 / 1) chance to get the items.
	 *		2 = 50% (1 / 2) chance.
	 *		10 = 10% (1 / 10) chance to get the items.
	 *		50 = 2% (1 / 50) chance.
	 *		250 = 0.4% (1 / 250) chance.
	 * Try typing '1/x' for calculating high percentage odds, e.g. 1/0.85 for an 85% chance.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bUseOdds", ClampMin = "1"))
	float Odds = 1.f;

	void UpdateDerivedValue();

	/** Perform a random test and return true if it passes the probability test. */
	bool RandomCheck() const;
};


/**
 * Defines the items and quantities to spawn
 * for a selected entry in a game item drop.
 */
USTRUCT(BlueprintType)
struct FGameItemDropContent
{
	GENERATED_BODY()

	virtual ~FGameItemDropContent() = default;

	/* The probability that any items will be given. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayPriority = -1))
	FGameItemDropProbability Probability;

	/** Perform a random probability test, then select and return items for this content if passed. */
	virtual void CheckAndSelectItems(TArray<FGameItemDefStack>& OutItems) const;

	/** Select and return items for this content. */
	virtual void SelectItems(TArray<FGameItemDefStack>& OutItems) const;

	/** Called when the owning data table row has changed. */
	virtual void OnDataChanged();
};

/** An explicit item and count. */
USTRUCT(BlueprintType, DisplayName = "Item")
struct FGameItemDropContent_Item : public FGameItemDropContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowAbstract = false))
	TSoftClassPtr<UGameItemDef> ItemDef = nullptr;

	virtual void SelectItems(TArray<FGameItemDefStack>& OutItems) const override;
};


/** All items from an item set. */
USTRUCT(BlueprintType, DisplayName = "Item Set")
struct FGameItemDropContent_ItemSet : public FGameItemDropContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UGameItemSet> ItemSet = nullptr;

	virtual void SelectItems(TArray<FGameItemDefStack>& OutItems) const override;
};


/** Selects entries from an item set as the content for this drop. */
USTRUCT(BlueprintType, DisplayName = "Item Set Entry")
struct FGameItemDropContent_ItemSetEntry : public FGameItemDropContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UGameItemSet> ItemSet = nullptr;

	/** The class that defines logic for how to select an entry from the drop table. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameItemSetEntrySelector> SelectorClass = UGameItemSetEntrySelector_Random::StaticClass();

	virtual void SelectItems(TArray<FGameItemDefStack>& OutItems) const override;
};


/** Selects entries from another drop table as the content for this drop. */
USTRUCT(BlueprintType, DisplayName = "Drop Table Entry")
struct FGameItemDropContent_DropTableEntry : public FGameItemDropContent
{
	GENERATED_BODY()

	/** The drop table from which to select entries. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType = "/Script/GameItems.GameItemDropTableRow"))
	FDataTableRowHandle DropTableRow;

	virtual void SelectItems(TArray<FGameItemDefStack>& OutItems) const override;
};
