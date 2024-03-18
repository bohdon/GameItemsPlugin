// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "Engine/DataTable.h"
#include "GameItemDropTableRow.generated.h"

struct FGameItemDropContent;


/**
 * Row struct for a game item drop table.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemDropTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * The number of times to select items from Content.
	 * If content represents multiple items, this effectively acts as a multiplier.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = "1"))
	int32 Count = 1;

	/** The items and quantities to create if the entry is selected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BaseStruct = "/Script/GameItems.GameItemDropContent", ExcludeBaseStruct))
	TArray<TInstancedStruct<FGameItemDropContent>> Content;

	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override;
};
