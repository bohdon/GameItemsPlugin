// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerRule_TagRequirements.h"

#include "GameItem.h"
#include "GameItemDef.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerRule_TagRequirements)


void UGameItemContainerRule_TagRequirements::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerRule_TagRequirements, RequireTags, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerRule_TagRequirements, IgnoreTags, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerRule_TagRequirements, Query, SharedParams);
}

bool UGameItemContainerRule_TagRequirements::CanContainItem_Implementation(const UGameItem* Item) const
{
	return CanContainItemByDef_Implementation(Item->GetItemDef());
}

bool UGameItemContainerRule_TagRequirements::CanContainItemByDef_Implementation(TSubclassOf<UGameItemDef> ItemDef) const
{
	const FGameplayTagContainer& ItemTags = GetDefault<UGameItemDef>(ItemDef)->OwnedTags;
	return ItemTags.HasAll(RequireTags) && !ItemTags.HasAny(IgnoreTags) && (Query.IsEmpty() || Query.Matches(ItemTags));
}