// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSubsystem.h"

#include "GameItem.h"
#include "GameItemDef.h"


UGameItem* UGameItemSubsystem::CreateGameItem(UObject* Outer, TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	if (!ItemDef)
	{
		return nullptr;
	}

	UGameItem* NewItem = NewObject<UGameItem>(Outer);
	NewItem->SetItemDef(ItemDef);
	NewItem->SetCount(Count);

	// notify all fragments to configure the new item if needed
	for (const UGameItemFragment* Fragment : GetDefault<UGameItemDef>(ItemDef)->Fragments)
	{
		if (Fragment)
		{
			Fragment->OnItemCreated(NewItem);
		}
	}

	return NewItem;
}
