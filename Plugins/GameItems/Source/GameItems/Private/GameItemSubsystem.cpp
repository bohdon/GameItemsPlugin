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

UGameItem* UGameItemSubsystem::CreateItem(UObject* Outer, TSubclassOf<UGameItemDef> ItemDef, int32 Count)
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

TArray<UGameItem*> UGameItemSubsystem::CreateItemInContainer(UGameItemContainer* Container, TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	if (!Container || !Container->GetOwner())
	{
		return TArray<UGameItem*>();
	}

	UGameItem* NewItem = CreateItem(Container->GetOwner(), ItemDef, Count);
	if (!NewItem)
	{
		return TArray<UGameItem*>();
	}

	TArray<UGameItem*> AddedItems = Container->AddItem(NewItem);
	return AddedItems;
}

UGameItem* UGameItemSubsystem::DuplicateItem(UObject* Outer, UGameItem* Item, int32 Count)
{
	if (!Item)
	{
		return nullptr;
	}

	const int32 NewCount = Count > 0 ? Count : Item->GetCount();
	UGameItem* NewItem = CreateItem(Outer, Item->GetItemDef(), NewCount);
	if (!NewItem)
	{
		return nullptr;
	}

	NewItem->CopyItemProperties(Item);

	return NewItem;
}

UGameItem* UGameItemSubsystem::SplitItem(UObject* Outer, UGameItem* Item, int32 Count)
{
	if (!Item || Item->GetCount() <= Count)
	{
		return nullptr;
	}

	Item->SetCount(Item->GetCount() - Count);
	UGameItem* NewItem = DuplicateItem(Outer, Item, Count);
	return NewItem;
}

TArray<UGameItem*> UGameItemSubsystem::MoveItem(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, UGameItem* Item, bool bAllowPartial)
{
	if (!FromContainer || !ToContainer || !FromContainer->Contains(Item))
	{
		return TArray<UGameItem*>();
	}

	const FGameItemContainerAddPlan Plan = ToContainer->CheckAddItem(Item);
	if (Plan.DeltaCount == 0)
	{
		// nothing to move
		return TArray<UGameItem*>();
	}

	// don't allow partial move
	if (!bAllowPartial && !Plan.bWillAddFullAmount)
	{
		return TArray<UGameItem*>();
	}

	// split the item if needed
	UGameItem* ItemToAdd = Item;
	if (Plan.RemainderCount > 0)
	{
		check(Item->GetCount() > Plan.DeltaCount);
		ItemToAdd = SplitItem(ToContainer->GetOwner(), Item, Plan.DeltaCount);
		check(ItemToAdd);
	}
	else
	{
		// remove the whole item
		FromContainer->RemoveItem(Item);
	}

	// add the item
	TArray<UGameItem*> Result = ToContainer->AddItem(ItemToAdd);

	return Result;
}

TArray<UGameItem*> UGameItemSubsystem::MoveItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer,
                                                 TArray<UGameItem*> Items, bool bAllowPartial)
{
	TArray<UGameItem*> Result;
	for (UGameItem* Item : Items)
	{
		TArray<UGameItem*> ItemResult = MoveItem(FromContainer, ToContainer, Item, bAllowPartial);
		Result.Append(ItemResult);
	}
	return Result;
}

TArray<UGameItem*> UGameItemSubsystem::MoveAllItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, bool bAllowPartial)
{
	if (FromContainer)
	{
		const TArray<UGameItem*> Items = FromContainer->GetAllItems();
		return MoveItems(FromContainer, ToContainer, Items, bAllowPartial);
	}
	return TArray<UGameItem*>();
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
