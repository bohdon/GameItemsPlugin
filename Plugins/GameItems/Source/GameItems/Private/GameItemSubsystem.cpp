// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSubsystem.h"

#include "DisplayDebugHelpers.h"
#include "GameItem.h"
#include "GameItemContainer.h"
#include "GameItemContainerComponent.h"
#include "GameItemContainerInterface.h"
#include "GameItemDef.h"
#include "GameItemsModule.h"
#include "GameItemStatics.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemSubsystem)


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

TArray<UGameItem*> UGameItemSubsystem::CreateGameItemInContainer(UGameItemContainer* Container, TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	if (!Container || !Container->GetOwner())
	{
		return TArray<UGameItem*>();
	}

	UGameItem* NewItem = CreateGameItem(Container->GetOwner(), ItemDef, Count);
	if (!NewItem)
	{
		return TArray<UGameItem*>();
	}

	TArray<UGameItem*> AddedItems = Container->AddItem(NewItem);
	return AddedItems;
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

TArray<UGameItemContainer*> UGameItemSubsystem::GetAllContainersForActor(AActor* Actor) const
{
	if (const IGameItemContainerInterface* ContainerInterface = GetContainerInterfaceForActor(Actor))
	{
		return ContainerInterface->GetAllItemContainers();
	}
	return TArray<UGameItemContainer*>();
}

UGameItemContainer* UGameItemSubsystem::GetContainerForActor(AActor* Actor, FGameplayTag ContainerId) const
{
	if (const IGameItemContainerInterface* ContainerInterface = GetContainerInterfaceForActor(Actor))
	{
		return ContainerInterface->GetItemContainer(ContainerId);
	}

	return nullptr;
}

UGameItemContainer* UGameItemSubsystem::GetDefaultContainerForActor(AActor* Actor) const
{
	if (const IGameItemContainerInterface* ContainerInterface = GetContainerInterfaceForActor(Actor))
	{
		return ContainerInterface->GetDefaultItemContainer();
	}
	return nullptr;
}

IGameItemContainerInterface* UGameItemSubsystem::GetContainerInterfaceForActor(AActor* Actor) const
{
	if (Actor)
	{
		if (IGameItemContainerInterface* ContainerInterface = Cast<IGameItemContainerInterface>(Actor))
		{
			return ContainerInterface;
		}
		if (UGameItemContainerComponent* ContainerComponent = Actor->FindComponentByClass<UGameItemContainerComponent>())
		{
			return ContainerComponent;
		}
	}
	return nullptr;
}

void UGameItemSubsystem::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	// showdebug GameItems
	if (!DisplayInfo.IsDisplayOn(ShowDebugNames::GameItems))
	{
		return;
	}

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(TEXT("GAME ITEMS"));

	// display debug info for all containers of the target actor
	TArray<UGameItemContainer*> Containers = GetAllContainersForActor(HUD->GetCurrentDebugTargetActor());
	for (const UGameItemContainer* Container : Containers)
	{
		if (!IsValid(Container))
		{
			continue;
		}

		Container->DisplayDebug(Canvas, DisplayInfo, YL, YPos);
	}
}
