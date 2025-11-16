// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSubsystem.h"

#include "DisplayDebugHelpers.h"
#include "GameItem.h"
#include "GameItemContainer.h"
#include "GameItemContainerComponent.h"
#include "GameItemContainerComponentInterface.h"
#include "GameItemContainerInterface.h"
#include "GameItemDef.h"
#include "GameItemsModule.h"
#include "GameItemStatics.h"
#include "DropTable/GameItemDropTableRow.h"
#include "Engine/Canvas.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
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
	if (!Container || !Container->GetItemOuter())
	{
		return TArray<UGameItem*>();
	}

	UGameItem* NewItem = CreateItem(Container->GetItemOuter(), ItemDef, Count);
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

TArray<UGameItem*> UGameItemSubsystem::MoveItem(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer,
                                                UGameItem* Item, int32 TargetSlot, bool bAllowPartial)
{
	if (!FromContainer || !ToContainer || !FromContainer->Contains(Item))
	{
		return TArray<UGameItem*>();
	}

	const FGameItemContainerAddPlan Plan = ToContainer->CheckAddItem(Item, TargetSlot, FromContainer);
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
		ItemToAdd = SplitItem(ToContainer->GetItemOuter(), Item, Plan.DeltaCount);
		check(ItemToAdd);
	}
	else
	{
		// remove the whole item
		FromContainer->RemoveItem(Item);
	}

	// add the item
	TArray<UGameItem*> Result = ToContainer->AddItem(ItemToAdd, TargetSlot);

	return Result;
}

TArray<UGameItem*> UGameItemSubsystem::MoveItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer,
                                                 TArray<UGameItem*> Items, bool bAllowPartial)
{
	TArray<UGameItem*> Result;
	for (UGameItem* Item : Items)
	{
		TArray<UGameItem*> ItemResult = MoveItem(FromContainer, ToContainer, Item, -1, bAllowPartial);
		Result.Append(ItemResult);
	}
	return Result;
}

TArray<UGameItem*> UGameItemSubsystem::MoveAllItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, bool bAllowPartial)
{
	if (FromContainer)
	{
		// TODO: optimize
		const TMap<int32, UGameItem*> ItemsBySlot = FromContainer->GetAllItems();
		TArray<UGameItem*> Items;
		ItemsBySlot.GenerateValueArray(Items);
		return MoveItems(FromContainer, ToContainer, Items, bAllowPartial);
	}
	return TArray<UGameItem*>();
}

TArray<FGameItemDefStack> UGameItemSubsystem::SelectItemsFromDropTable(const FGameItemDropContext& Context, FDataTableRowHandle DropTableEntry)
{
	static FString ContextString(TEXT("UGameItemSubsystem::SelectItemsFromDropTable"));
	const FGameItemDropTableRow* Row = DropTableEntry.GetRow<FGameItemDropTableRow>(ContextString);
	if (!Row)
	{
		return TArray<FGameItemDefStack>();
	}

	TArray<FGameItemDefStack> Result;
	UGameItemStatics::SelectItemsFromDropTableRow(Context, *Row, Result);
	return Result;
}

TArray<UGameItem*> UGameItemSubsystem::CreateItemsFromDropTable(UObject* Outer, const FGameItemDropContext& Context, FDataTableRowHandle DropTableEntry)
{
	TArray<FGameItemDefStack> Stacks = SelectItemsFromDropTable(Context, DropTableEntry);

	TArray<UGameItem*> Result;
	for (const FGameItemDefStack& Stack : Stacks)
	{
		if (UGameItem* NewItem = CreateItem(Outer, Stack.ItemDef, Stack.Count))
		{
			Result.Add(NewItem);
		}
	}
	return Result;
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

UGameItemContainerComponent* UGameItemSubsystem::GetContainerComponentForActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return nullptr;
	}
	if (const IGameItemContainerComponentInterface* ComponentInterface = Cast<IGameItemContainerComponentInterface>(Actor))
	{
		return ComponentInterface->GetItemContainerComponent();
	}

	return Actor->FindComponentByClass<UGameItemContainerComponent>();
}

TArray<UGameItemContainer*> UGameItemSubsystem::GetAllContainersForActor(const AActor* Actor) const
{
	if (const IGameItemContainerInterface* ContainerInterface = GetContainerInterfaceForActor(Actor))
	{
		return ContainerInterface->GetAllItemContainers();
	}
	return TArray<UGameItemContainer*>();
}

UGameItemContainer* UGameItemSubsystem::GetContainerForActor(const AActor* Actor, FGameplayTag ContainerId) const
{
	if (const IGameItemContainerInterface* ContainerInterface = GetContainerInterfaceForActor(Actor))
	{
		return ContainerInterface->GetItemContainer(ContainerId);
	}

	return nullptr;
}

UGameItemContainer* UGameItemSubsystem::GetDefaultContainerForActor(const AActor* Actor) const
{
	if (const IGameItemContainerInterface* ContainerInterface = GetContainerInterfaceForActor(Actor))
	{
		return ContainerInterface->GetDefaultItemContainer();
	}
	return nullptr;
}

const IGameItemContainerInterface* UGameItemSubsystem::GetContainerInterfaceForActor(const AActor* Actor) const
{
	if (const IGameItemContainerInterface* ContainerInterface = Cast<IGameItemContainerInterface>(Actor))
	{
		return ContainerInterface;
	}

	// fallback to finding a container component
	return GetContainerComponentForActor(Actor);
}

void UGameItemSubsystem::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	// showdebug GameItems
	if (!DisplayInfo.IsDisplayOn(ShowDebugNames::GameItems))
	{
		return;
	}

	FString NetSuffix;
	switch (HUD->GetNetMode())
	{
	case NM_DedicatedServer:
	case NM_ListenServer:
		NetSuffix = FString::Printf(TEXT("(Server)"));
		break;
	case NM_Client:
		NetSuffix = FString::Printf(TEXT("(Client %d)"), UE::GetPlayInEditorID());
		break;
	case NM_Standalone:
	default: ;
	}

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;
	DisplayDebugManager.SetDrawColor(FColor::Yellow);

	DisplayDebugManager.DrawString(FString::Printf(TEXT("GAME ITEMS %s"), *NetSuffix));

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

UGameItemSubsystem* UGameItemSubsystem::GetGameItemSubsystem(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World ? UGameInstance::GetSubsystem<UGameItemSubsystem>(World->GetGameInstance()) : nullptr;
}
