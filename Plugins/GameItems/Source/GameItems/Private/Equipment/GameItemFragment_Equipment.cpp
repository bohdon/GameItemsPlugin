// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameItemFragment_Equipment.h"

#include "GameItem.h"
#include "GameItemContainer.h"
#include "Conditions/GameItemConditionSchema.h"


UGameItemFragment_Equipment::UGameItemFragment_Equipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Condition.SetSchemaClass(UGameItemConditionSchema::StaticClass());
}

FGameEquipmentSpec UGameItemFragment_Equipment::CreateEquipmentSpecForItem(UGameItem* Item) const
{
	// copy all tag stats from the item, this allows applying equipment on server,
	// even from local-only items that do not replicate.
	const TArray<FGameItemTagStack>& TagStats = Item->GetTagStatsContainer().Stacks;

	// also include all container tags where the item is slotted.
	FGameplayTagContainer ContextTags;
	for (const UGameItemContainer* Container : Item->GetContainers())
	{
		ContextTags.AddTag(Container->GetContainerId());
	}

	return Item ? FGameEquipmentSpec(EquipmentDef, TagStats, ContextTags) : FGameEquipmentSpec();
}
