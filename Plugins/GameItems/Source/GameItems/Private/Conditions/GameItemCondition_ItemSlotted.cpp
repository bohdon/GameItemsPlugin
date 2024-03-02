// Copyright Bohdon Sayre, All Rights Reserved.


#include "Conditions/GameItemCondition_ItemSlotted.h"

#include "GameItem.h"
#include "GameItemContainer.h"
#include "WorldConditionContext.h"
#include "Conditions/GameItemConditionSchema.h"

#define LOCTEXT_NAMESPACE "GameItems"


FText FGameItemCondition_ItemSlotted::GetDescription() const
{
	return FText::Format(LOCTEXT("ItemSlottedConditionDesc", "Slotted in {0}"), FText::FromString(ContainerTags.ToStringSimple()));
}

bool FGameItemCondition_ItemSlotted::Initialize(const UWorldConditionSchema& Schema)
{
	const UGameItemConditionSchema* ItemSchema = Cast<UGameItemConditionSchema>(&Schema);
	if (!ItemSchema)
	{
		return false;
	}

	bCanCacheResult = Schema.GetContextDataTypeByRef(ItemSchema->GetTargetItemRef()) == EWorldConditionContextDataType::Persistent;

	return true;
}

FWorldConditionResult FGameItemCondition_ItemSlotted::IsTrue(const FWorldConditionContext& Context) const
{
	const UGameItemConditionSchema* ItemSchema = Cast<UGameItemConditionSchema>(Context.GetSchema());
	const UGameItem* const TargetItem = Context.GetContextDataPtr<UGameItem>(ItemSchema->GetTargetItemRef());

	FWorldConditionResult Result(EWorldConditionResultValue::IsFalse, bCanCacheResult);
	if (!TargetItem)
	{
		return Result;
	}

	const TArray<UGameItemContainer*> Containers = TargetItem->GetContainers();
	for (const UGameItemContainer* Container : Containers)
	{
		if (Container->GetOwnedTags().HasAny(ContainerTags))
		{
			Result.Value = EWorldConditionResultValue::IsTrue;
			break;
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
