// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSubsystem.h"

#include "DisplayDebugHelpers.h"
#include "GameItem.h"
#include "GameItemContainerComponent.h"
#include "GameItemDef.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"


void UGameItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	AHUD::OnShowDebugInfo.AddUObject(this, &UGameItemSubsystem::OnShowDebugInfo);
}

void UGameItemSubsystem::Deinitialize()
{
	AHUD::OnShowDebugInfo.RemoveAll(this);
}

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

UGameItem* UGameItemSubsystem::DuplicateGameItem(UObject* Outer, UGameItem* Item)
{
	if (!Item)
	{
		return nullptr;
	}

	UGameItem* NewItem = CreateGameItem(Outer, Item->GetItemDef(), Item->GetCount());
	if (!NewItem)
	{
		return nullptr;
	}

	NewItem->CopyItemProperties(Item);

	return NewItem;
}

const UGameItemFragment* UGameItemSubsystem::FindFragment(TSubclassOf<UGameItemDef> ItemDef, TSubclassOf<UGameItemFragment> FragmentClass) const
{
	if (!ItemDef || !FragmentClass)
	{
		return nullptr;
	}

	const UGameItemDef* ItemDefCDO = GetDefault<UGameItemDef>(ItemDef);
	return ItemDefCDO->FindFragment(FragmentClass);
}

TArray<UGameItemContainerComponent*> UGameItemSubsystem::GetAllContainers(AActor* Actor) const
{
	TArray<UGameItemContainerComponent*> Result;
	Actor->GetComponents<UGameItemContainerComponent>(Result);
	return Result;
}

UGameItemContainerComponent* UGameItemSubsystem::FindContainerByTag(AActor* Actor, FGameplayTag IdTag) const
{
	TArray<UGameItemContainerComponent*> AllContainers = GetAllContainers(Actor);
	for (UGameItemContainerComponent* Container : AllContainers)
	{
		if (Container->IdTag == IdTag)
		{
			return Container;
		}
	}
	return nullptr;
}

void UGameItemSubsystem::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	// showdebug GameItems
	static const FName NAME_GameItems(TEXT("GameItems"));
	if (DisplayInfo.IsDisplayOn(NAME_GameItems))
	{
		FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
		DisplayDebugManager.SetDrawColor(FColor::Yellow);
		DisplayDebugManager.DrawString(TEXT("GAME ITEMS"));

		// display debug info for all containers of the target actor
		TInlineComponentArray<UGameItemContainerComponent*> Containers(HUD->GetCurrentDebugTargetActor());
		for (const UGameItemContainerComponent* Container : Containers)
		{
			if (!IsValid(Container))
			{
				continue;
			}

			Container->DisplayDebug(Canvas, DisplayInfo, YL, YPos);
		}
	}
}
