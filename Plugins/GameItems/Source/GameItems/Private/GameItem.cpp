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

void UGameItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemDef, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Count, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TagStats, Params);
}

const UGameItemDef* UGameItem::GetItemDefCDO() const
{
	return ItemDef ? GetDefault<UGameItemDef>(ItemDef) : nullptr;
}

void UGameItem::SetItemDef(TSubclassOf<UGameItemDef> NewItemDef)
{
	if (ItemDef != NewItemDef)
	{
		ItemDef = NewItemDef;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemDef, this);
	}
}

const FGameplayTagContainer& UGameItem::GetOwnedTags() const
{
	const UGameItemDef* ItemDefCDO = GetItemDefCDO();
	return ItemDefCDO ? ItemDefCDO->OwnedTags : FGameplayTagContainer::EmptyContainer;
}

void UGameItem::SetCount(int32 NewCount)
{
	if (Count != NewCount)
	{
		const int32 OldCount = Count;
		Count = NewCount;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Count, this);

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
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TagStats, this);
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
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TagStats, this);
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
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Count, this);
	TagStats = Item->TagStats;
	TagStats.MarkArrayDirty();
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TagStats, this);
}

TArray<UGameItemContainer*> UGameItem::GetContainers() const
{
	TArray<UGameItemContainer*> Result;
	Algo::TransformIf(Containers, Result,
	                  [](const TWeakObjectPtr<UGameItemContainer>& Container) { return Container.IsValid(); },
	                  [](const TWeakObjectPtr<UGameItemContainer>& Container) { return Container.Get(); });
	return Result;
}

FString UGameItem::GetDebugString() const
{
	FString ItemDefName = GetNameSafe(ItemDef);
	ItemDefName.RemoveFromEnd(TEXT("_C"));
	return FString::Printf(TEXT("%s (%s x %d)"), *GetName(), *ItemDefName, Count);
}