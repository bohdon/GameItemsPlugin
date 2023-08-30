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

const UGameItemDef* UGameItem::GetItemDefCDO() const
{
	return ItemDef ? GetDefault<UGameItemDef>(ItemDef) : nullptr;
}

void UGameItem::SetItemDef(TSubclassOf<UGameItemDef> NewItemDef)
{
	ItemDef = NewItemDef;
}

void UGameItem::SetCount(int32 NewCount)
{
	if (Count != NewCount)
	{
		const int32 OldCount = Count;
		Count = NewCount;

		OnCountChangedEvent.Broadcast(NewCount, OldCount);
	}
}

void UGameItem::AddTagStat(FGameplayTag Tag, int32 DeltaValue)
{
	if (DeltaValue <= 0)
	{
		return;
	}

	const int32 OldValue = TagStats.GetStackCount(Tag);
	TagStats.AddStack(Tag, DeltaValue);
	const int32 NewValue = TagStats.GetStackCount(Tag);

	if (OldValue != NewValue)
	{
		OnTagStatChangedEvent.Broadcast(Tag, NewValue, OldValue);
	}
}

void UGameItem::RemoveTagStat(FGameplayTag Tag, int32 DeltaValue)
{
	if (DeltaValue <= 0)
	{
		return;
	}

	const int32 OldValue = TagStats.GetStackCount(Tag);
	TagStats.RemoveStack(Tag, DeltaValue);
	const int32 NewValue = TagStats.GetStackCount(Tag);

	if (OldValue != NewValue)
	{
		OnTagStatChangedEvent.Broadcast(Tag, NewValue, OldValue);
	}
}

int32 UGameItem::GetTagStat(FGameplayTag Tag) const
{
	return TagStats.GetStackCount(Tag);
}

bool UGameItem::IsMatching(const UGameItem* Item) const
{
	// TODO: add item fragment or item def properties for requiring matching tag stats
	return Item && Item->ItemDef == ItemDef;
}

void UGameItem::CopyItemProperties(const UGameItem* Item)
{
	Count = Item->Count;
	TagStats = Item->TagStats;
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
