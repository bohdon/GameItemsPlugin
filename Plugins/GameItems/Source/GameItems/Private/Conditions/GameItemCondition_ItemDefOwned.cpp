// Copyright Bohdon Sayre, All Rights Reserved.


#include "Conditions/GameItemCondition_ItemDefOwned.h"

#include "GameItemContainerComponent.h"
#include "GameItemDef.h"
#include "GameItemSubsystem.h"
#include "WorldConditionContext.h"
#include "Conditions/GameItemConditionSchema.h"
#include "GameFramework/Actor.h"

#define LOCTEXT_NAMESPACE "GameItems"


#if WITH_EDITOR
FText FGameItemCondition_ItemDefOwned::GetDescription() const
{
	return FText::Format(LOCTEXT("ItemOwnedConditionDesc", "Owns {0}"), FText::FromString(GetNameSafe(ItemDef)));
}
#endif

bool FGameItemCondition_ItemDefOwned::Initialize(const UWorldConditionSchema& Schema)
{
	const UGameItemConditionSchema* ItemSchema = Cast<UGameItemConditionSchema>(&Schema);
	if (!ItemSchema)
	{
		return false;
	}

	bCanCacheResult = Schema.GetContextDataTypeByRef(ItemSchema->GetTargetActorRef()) == EWorldConditionContextDataType::Persistent;

	return true;
}

FWorldConditionResult FGameItemCondition_ItemDefOwned::IsTrue(const FWorldConditionContext& Context) const
{
	const UGameItemConditionSchema* ItemSchema = Cast<UGameItemConditionSchema>(Context.GetSchema());
	const AActor* const TargetActor = Context.GetContextDataPtr<AActor>(ItemSchema->GetTargetActorRef());

	FWorldConditionResult Result(EWorldConditionResultValue::IsFalse, bCanCacheResult);
	if (!TargetActor)
	{
		return Result;
	}

	const UGameItemSubsystem* const ItemSubsystem = Context.GetContextDataPtr<UGameItemSubsystem>(ItemSchema->GetSubsystemRef());
	if (const UGameItemContainerComponent* ContainerComponent = ItemSubsystem->GetContainerComponentForActor(TargetActor))
	{
		if (ContainerComponent->GetTotalMatchingItemCountByDef(ItemDef) > 0)
		{
			Result.Value = EWorldConditionResultValue::IsTrue;
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
