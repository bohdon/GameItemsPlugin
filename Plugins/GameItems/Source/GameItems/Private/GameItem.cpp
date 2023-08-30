// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItem.h"

#include "GameItemDef.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItem)

class FLifetimeProperty;

UGameItem::UGameItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  Count(0)
{
}

void UGameItem::SetItemDef(TSubclassOf<UGameItemDef> NewItemDef)
{
	ItemDef = NewItemDef;
}

void UGameItem::SetCount(int32 NewCount)
{
	if (Count != NewCount)
	{
		Count = NewCount;
		// TODO: broadcast event
	}
}

void UGameItem::AddTagStat(FGameplayTag Tag, int32 DeltaCount)
{
	if (DeltaCount <= 0)
	{
		return;
	}

	const int32 OldCount = TagStats.GetStackCount(Tag);
	TagStats.AddStack(Tag, DeltaCount);
	const int32 NewCount = TagStats.GetStackCount(Tag);

	if (OldCount != NewCount)
	{
		OnTagStatChangedEvent.Broadcast(Tag, NewCount, OldCount);
	}
}

void UGameItem::RemoveTagStat(FGameplayTag Tag, int32 DeltaCount)
{
	if (DeltaCount <= 0)
	{
		return;
	}

	const int32 OldCount = TagStats.GetStackCount(Tag);
	TagStats.RemoveStack(Tag, DeltaCount);
	const int32 NewCount = TagStats.GetStackCount(Tag);

	if (OldCount != NewCount)
	{
		OnTagStatChangedEvent.Broadcast(Tag, NewCount, OldCount);
	}
}

int32 UGameItem::GetTagStat(FGameplayTag Tag) const
{
	return TagStats.GetStackCount(Tag);
}

FString UGameItem::ToDebugString() const
{
	return FString::Printf(TEXT("%s (%sx%d)"), *GetName(), *GetNameSafe(ItemDef), Count);
}

void UGameItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemDef);
	DOREPLIFETIME(ThisClass, Count);
	DOREPLIFETIME(ThisClass, TagStats);
}
