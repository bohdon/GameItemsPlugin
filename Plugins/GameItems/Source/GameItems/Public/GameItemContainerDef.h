// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"
#include "UObject/ObjectSaveContext.h"
#include "GameItemContainerDef.generated.h"

class UGameItemContainer;
class UGameItemContainerRule;
class UGameItemSet;


/**
 * Defines the settings and rules for a game item container.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const)
class GAMEITEMS_API UGameItemContainerDef : public UObject
{
	GENERATED_BODY()

public:
	UGameItemContainerDef(const FObjectInitializer& ObjectInitializer);

	/** The container class to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, Category = "GameItemContainer")
	TSubclassOf<UGameItemContainer> ContainerClass;

	/** Tags that this container has. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItemContainer")
	FGameplayTagContainer OwnedTags;

	/** Limit the number of items/stacks that can be in this container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (InlineEditConditionToggle), Category = "GameItemContainer")
	bool bLimitSlots;

	/** The limited number of slots available in this container, when bLimitSlots is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bLimitSlots"), Category = "GameItemContainer")
	int32 SlotCount;

	/** Automatically combine and stack items that are added to the container (unless explicitly added to an empty slot). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItemContainer")
	bool bAutoStack;

	/** The items that should be in this container by default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItemContainer")
	TArray<FGameItemDefStack> DefaultItems;

	/** The item sets that should be in this container by default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItemContainer")
	TArray<TObjectPtr<UGameItemSet>> DefaultItemSets;

	/** The item drop table row to use for filling this container up with starting items. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType = "/Script/GameItems.GameItemDropTableRow"), Category = "GameItemContainer")
	FDataTableRowHandle DefaultDropContent;

	/** Rules and limitations specific to this container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "GameItemContainer")
	TArray<TObjectPtr<UGameItemContainerRule>> Rules;

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};
