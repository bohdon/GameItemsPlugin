// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "Engine/DataAsset.h"
#include "UObject/ObjectSaveContext.h"
#include "GameItemSet.generated.h"

class UGameItemFragment;
class UGameItemSet;


/**
 * An editor-only class for automatically filling game item sets by searching
 * for matching item definitions.
 */
UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew)
class GAMEITEMS_API UGameItemSetAutoFill : public UObject
{
	GENERATED_BODY()

public:
	/** Include this asset's directory in the directories to search. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Fill")
	bool bSearchCurrentDirectory;

	/** Asset directories to search for game item definitions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (LongPackageName), Category = "Auto Fill")
	TArray<FDirectoryPath> SearchDirectories;

	/** Items must have all of these tags to be included. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Fill")
	FGameplayTagContainer RequireTags;

	/** Items must have none of these tags to be included. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Fill")
	FGameplayTagContainer IgnoreTags;

	/** Items must match this tag query to be included. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Fill")
	FGameplayTagQuery TagQuery;

	/** Items must have this item fragment to be included. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Fill")
	TArray<TSubclassOf<UGameItemFragment>> RequireFragments;

	/** Fill an item set with items. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = false)
	void FillSet(UGameItemSet* ItemSet) const;

	/** Return true if an item definition should be included in the set. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
	bool ShouldIncludeItem(TSubclassOf<UGameItemDef> ItemDef) const;
};


/**
 * A collection of game items, as definitions and quantities,
 * e.g. for use in predefining containers or groups of items to give out.
 */
UCLASS(BlueprintType, Blueprintable)
class GAMEITEMS_API UGameItemSet : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	/** If set, will automatically fill this item set by searching for matching item definition assets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Items")
	TObjectPtr<UGameItemSetAutoFill> AutoFillRule;
#endif

	/** The items in the set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "AutoFillRule == nullptr", TitleProperty = "{Count} {ItemDef}"), Category = "Items")
	TArray<FGameItemDefStack> Items;

	/** Add all items in this set to a container. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	virtual void AddToContainer(UGameItemContainer* Container) const;

#if WITH_EDITOR

public:
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
};
