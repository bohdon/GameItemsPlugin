// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerStockRule_Tags.h"

#include "GameItem.h"
#include "GameItemDef.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerStockRule_Tags)


UGameItemContainerStockRule_Tags::UGameItemContainerStockRule_Tags()
{
}

void UGameItemContainerStockRule_Tags::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ContainerLimitsArray, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StackLimitsArray, Params);
}

#if WITH_EDITOR
void UGameItemContainerStockRule_Tags::PostLoad()
{
	Super::PostLoad();

	// migrate TMap limits
	if (!ContainerLimits_DEPRECATED.IsEmpty())
	{
		MarkPackageDirty();
		if (ContainerLimitsArray.IsEmpty())
		{
			for (const TTuple<FGameplayTag, FGameItemCountLimit>& Elem : ContainerLimits_DEPRECATED)
			{
				ContainerLimitsArray.Emplace(Elem.Key, Elem.Value.bLimitCount, Elem.Value.MaxCount);
			}
		}
		ContainerLimits_DEPRECATED.Empty();
	}
	if (!StackLimits_DEPRECATED.IsEmpty())
	{
		MarkPackageDirty();
		if (StackLimitsArray.IsEmpty())
		{
			for (const TTuple<FGameplayTag, FGameItemCountLimit>& Elem : StackLimits_DEPRECATED)
			{
				StackLimitsArray.Emplace(Elem.Key, Elem.Value.bLimitCount, Elem.Value.MaxCount);
			}
		}
		StackLimits_DEPRECATED.Empty();
	}
}
#endif

int32 UGameItemContainerStockRule_Tags::GetItemMaxCount_Implementation(const UGameItem* Item) const
{
	return FindLimitForItem(Item, ContainerLimitsArray).GetMaxCount(-1);
}

int32 UGameItemContainerStockRule_Tags::GetItemStackMaxCount_Implementation(const UGameItem* Item) const
{
	return FindLimitForItem(Item, StackLimitsArray).GetMaxCount(-1);
}

FGameItemCountLimit UGameItemContainerStockRule_Tags::FindLimitForItem(const UGameItem* Item, const TArray<FGameItemTagCountLimit>& Limits)
{
	const UGameItemDef* ItemDefCDO = Item ? Item->GetItemDefCDO() : nullptr;
	if (!ItemDefCDO)
	{
		return FGameItemCountLimit();
	}

	for (const FGameItemTagCountLimit& Limit : Limits)
	{
		if (ItemDefCDO->OwnedTags.HasTag(Limit.ItemTag))
		{
			return FGameItemCountLimit(Limit);
		}
	}
	return FGameItemCountLimit();
}
