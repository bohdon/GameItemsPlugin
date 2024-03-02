// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemConditionBase.h"
#include "Templates/SubclassOf.h"
#include "GameItemCondition_ItemDefOwned.generated.h"

class UGameItemDef;

/**
 * A condition that is true when the target actor owns any of an item by definition.
 */
USTRUCT(meta=(DisplayName="Item Def Owned"))
struct FGameItemCondition_ItemDefOwned : public FGameItemConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Default")
	TSubclassOf<UGameItemDef> ItemDef;

protected:
#if WITH_EDITOR
	virtual FText GetDescription() const override;
#endif

	virtual bool Initialize(const UWorldConditionSchema& Schema) override;
	virtual FWorldConditionResult IsTrue(const FWorldConditionContext& Context) const override;
};
