// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "UObject/Object.h"
#include "GameItemContainerRule.generated.h"

class UGameItem;
class UGameItemContainer;


/**
 * Defines conditions and stock limitations for a game item container.
 * Rules can be stateful and are instantiated within each container they apply to.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class GAMEITEMS_API UGameItemContainerRule : public UObject
{
	GENERATED_BODY()

public:
	UGameItemContainerRule();

	/** Return the owning container. */
	UGameItemContainer* GetContainer() const { return Container; }

	/** Initialize the rule. Called when added to a container. */
	virtual void Initialize();

	/** Uninitialize the rule. Called when removed from a container. */
	virtual void Uninitialize();

	/** Return true if an item is allowed in the container. */
	UFUNCTION(BlueprintNativeEvent)
	bool CanContainItem(const UGameItem* Item) const;

	/** Return the maximum allowed count for an item, or -1 if unlimited. */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetItemMaxCount(const UGameItem* Item) const;

	/** Return the maximum allowed count for a single stack of an item, or -1 if unlimited. */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetItemStackMaxCount(const UGameItem* Item) const;

	virtual UWorld* GetWorld() const override;

protected:
	/** The owning container of this rule. */
	UPROPERTY(Transient, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UGameItemContainer> Container;
};


/**
 * Defines tag requirements defining which items are permitted in the container.
 */
UCLASS(DisplayName = "Tag Requirements")
class UGameItemContainerRule_TagRequirements : public UGameItemContainerRule
{
	GENERATED_BODY()

public:
	/** Items must have all of these tags to be permitted in the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Requirements")
	FGameplayTagContainer RequireTags;

	/** Items cannot have any of these tags to be permitted in the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Requirements")
	FGameplayTagContainer IgnoreTags;

	/** Items must match this query to be permitted in the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Requirements")
	FGameplayTagQuery Query;

	virtual bool CanContainItem_Implementation(const UGameItem* Item) const override;
};


/**
 * Specifies a flat limit for max count and stack max count of any items.
 */
UCLASS(DisplayName = "Simple Stock Rules")
class GAMEITEMS_API UGameItemContainerStockRule_Simple : public UGameItemContainerRule
{
	GENERATED_BODY()

public:
	UGameItemContainerStockRule_Simple();

	/** The stock rules for all items. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (ShowOnlyInnerProperties), Category = "Stock Rules")
	FGameItemStockRules StockRules;

	virtual int32 GetItemMaxCount_Implementation(const UGameItem* Item) const override;
	virtual int32 GetItemStackMaxCount_Implementation(const UGameItem* Item) const override;
};


/**
 * Specifies stock rules by item tags, where rules apply to the first tag that matches an item.
 */
UCLASS(DisplayName = "Tag Stock Rules")
class GAMEITEMS_API UGameItemContainerStockRule_Tags : public UGameItemContainerRule
{
	GENERATED_BODY()

public:
	UGameItemContainerStockRule_Tags();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tag Stock Rules")
	TMap<FGameplayTag, FGameItemStockRules> StockRules;

	FGameItemStockRules GetStockRulesForItem(const UGameItem* Item) const;

	virtual int32 GetItemMaxCount_Implementation(const UGameItem* Item) const override;
	virtual int32 GetItemStackMaxCount_Implementation(const UGameItem* Item) const override;
};


/**
 * Attempts to auto-slot an item whenever it is added to this container.
 * Requires the container to be in a UGameItemContainerComponent.
 */
UCLASS(DisplayName = "Auto-Slot")
class UGameItemContainerRule_AutoSlot : public UGameItemContainerRule
{
	GENERATED_BODY()

public:
	/** Context tags to pass when auto slotting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Slot")
	FGameplayTagContainer ContextTags;

	virtual void Initialize() override;
	virtual void Uninitialize() override;

	/** Return true if auto-slotting should be performed. */
	UFUNCTION(BlueprintNativeEvent)
	bool ShouldAutoSlot(UGameItem* Item) const;

protected:
	void OnItemAdded(UGameItem* Item);
};
