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
#include "GameFramework/Controller.h"
#include "GameFramework/HUD.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Sound/SoundConcurrency.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemSubsystem)


void UGameItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	AHUD::OnShowDebugInfo.AddUObject(this, &UGameItemSubsystem::OnShowDebugInfo);
}

void UGameItemSubsystem::Deinitialize()
{
	AHUD::OnShowDebugInfo.RemoveAll(this);
}

bool UGameItemSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// only create an instance if there is no overridden implementation
	return ChildClasses.Num() == 0;
}

UGameItem* UGameItemSubsystem::CreateItem(UObject* Outer, TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	if (!ItemDef)
	{
		return nullptr;
	}

	const UGameItemDef* ItemDefCDO = ItemDef->GetDefaultObject<UGameItemDef>();
	TSubclassOf<UGameItem> ItemClass = ItemDefCDO->ItemClass;
	if (!ensureMsgf(ItemClass, TEXT("%s.ItemClass is not valid"), *ItemDef->GetName()))
	{
		ItemClass = UGameItem::StaticClass();
	}

	UGameItem* NewItem = NewObject<UGameItem>(Outer, ItemClass);
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

UGameItem* UGameItemSubsystem::CreateItemFromSaveData(UObject* Outer, const FGameItemSaveData& ItemSaveData)
{
	const TSubclassOf<UGameItemDef> ItemDef = ItemSaveData.ItemDef.LoadSynchronous();
	if (!ItemDef)
	{
		UE_LOG(LogGameItems, Warning, TEXT("Failed to load item def from save data: %s (Outer: %s)"),
			*ItemSaveData.ItemDef.ToString(), *GetNameSafe(Outer));
		return nullptr;
	}

	UGameItem* NewItem = CreateItem(Outer, ItemDef);
	check(NewItem);

	// serialize item properties (including count)
	FMemoryReader MemReader(ItemSaveData.ByteData);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;
	NewItem->Serialize(Ar);

	return NewItem;
}

void UGameItemSubsystem::CreateItemInContainer(UGameItemContainer* Container, TSubclassOf<UGameItemDef> ItemDef, int32 Count, bool bWarn)
{
	if (!Container || !Container->GetItemOuter())
	{
		return;
	}

	// TODO: ensure we're creating items in a local owned container (e.g. not creating on client then attempting to add to server-owned or vice versa)

	// TODO: no way to check ahead of time and avoid creating items if we can't add?

	UGameItem* NewItem = CreateItem(Container->GetItemOuter(), ItemDef, Count);
	if (!NewItem)
	{
		return;
	}

	Container->AddItem(NewItem, -1, bWarn);
}

bool UGameItemSubsystem::HasItemStacks(UGameItemContainer* Container, TArray<FGameItemDefStack> ItemStacks) const
{
	for (const FGameItemDefStack& ItemStack : ItemStacks)
	{
		if (Container->GetTotalItemCountByDef(ItemStack.ItemDef) < ItemStack.Count)
		{
			return false;
		}
	}
	return true;
}

bool UGameItemSubsystem::RemoveItemStacks(UGameItemContainer* Container, TArray<FGameItemDefStack> ItemStacks, bool bAllowPartial) const
{
	if (!bAllowPartial)
	{
		if (!HasItemStacks(Container, ItemStacks))
		{
			return false;
		}
	}

	for (const FGameItemDefStack& ItemStack : ItemStacks)
	{
		Container->RemoveItemsByDef(ItemStack.ItemDef, ItemStack.Count);
	}

	return true;
}

UGameItem* UGameItemSubsystem::DuplicateItem(UObject* Outer, UGameItem* Item)
{
	if (!Item)
	{
		return nullptr;
	}

	UGameItem* NewItem = CreateItem(Outer, Item->GetItemDef());
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
	UGameItem* NewItem = DuplicateItem(Outer, Item);
	NewItem->SetCount(Count);
	return NewItem;
}

void UGameItemSubsystem::MoveItem(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, UGameItem* Item, int32 TargetSlot, bool bAllowPartial)
{
	if (!FromContainer || !ToContainer || !FromContainer->Contains(Item))
	{
		return;
	}
	if (FromContainer == ToContainer)
	{
		UE_LOG(LogGameItems, Verbose, TEXT("[%hs] Nothing to do, From and To container are the same: %s"),
			__FUNCTION__, *FromContainer->GetReadableName());
		return;
	}

	const FGameItemContainerAddPlan Plan = ToContainer->CheckAddItem(Item, TargetSlot, FromContainer);
	if (Plan.DeltaCount == 0)
	{
		// nothing to move
		return;
	}

	// don't allow partial move
	if (!bAllowPartial && !Plan.bWillAddFullAmount)
	{
		return;
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
	ToContainer->AddItem(ItemToAdd, TargetSlot);
}

void UGameItemSubsystem::MoveItems(
	UGameItemContainer* FromContainer,
	UGameItemContainer* ToContainer,
	TArray<UGameItem*> Items,
	bool bAllowPartial)
{
	for (UGameItem* Item : Items)
	{
		MoveItem(FromContainer, ToContainer, Item, -1, bAllowPartial);
	}
}

void UGameItemSubsystem::MoveAllItems(UGameItemContainer* FromContainer, UGameItemContainer* ToContainer, bool bAllowPartial)
{
	if (!FromContainer)
	{
		return;
	}

	// TODO: optimize
	const TMap<int32, UGameItem*> ItemsBySlot = FromContainer->GetAllItems();
	TArray<UGameItem*> Items;
	ItemsBySlot.GenerateValueArray(Items);
	MoveItems(FromContainer, ToContainer, Items, bAllowPartial);
}

bool UGameItemSubsystem::MoveSwapOrStackItem(UGameItemContainer* From, UGameItem* Item, UGameItemContainer* To, int32 ToSlot, bool bAllowPartial)
{
	if (!From || !To)
	{
		UE_LOG(LogGameItems, Warning, TEXT("[%hs] Invalid container. From: %s, To: %s"),
			__FUNCTION__, *GetNameSafe(From), *GetNameSafe(To));
		return false;
	}

	if (!To->IsValidSlot(ToSlot))
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s[%hs] Slot %d is not valid in To container: %s"),
			*UGameItemStatics::GetNetDebugPrefix(this), __func__, ToSlot, *To->GetReadableName());
		return false;
	}

	int32 FromSlot = From->GetItemSlot(Item);
	if (FromSlot == INDEX_NONE)
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s[%hs] Item %s not found in From container: %s"),
			*UGameItemStatics::GetNetDebugPrefix(this), __func__, *Item->GetDebugString(), *From->GetReadableName());
		return false;
	}

	const UGameItem* ToItem = To->GetItemAt(ToSlot);

	if (From == To)
	{
		// same container

		if (FromSlot == ToSlot)
		{
			// same slot
			UE_LOG(LogGameItems, VeryVerbose, TEXT("[%hs] Same slot, nothing to do: %s Slot: %d -> %d"),
				__FUNCTION__, *From->GetReadableName(), FromSlot, ToSlot);
			return false;
		}

		if (Item->IsMatching(ToItem) && !To->IsStackFull(ToSlot))
		{
			// TODO: if not allowing partial, and item cant fully fit in the target stack, fallback to swap
			// stack items
			To->StackItems(FromSlot, ToSlot, bAllowPartial);
		}
		else
		{
			// swap items in the container
			To->SwapItems(FromSlot, ToSlot);
		}
	}
	else if (To->IsChild())
	{
		// assign / replace item to a child container
		if (ToItem)
		{
			To->RemoveItemAt(ToSlot);
		}
		const int32 ExistingItemSlot = To->GetItemSlot(Item);
		if (ExistingItemSlot != INDEX_NONE)
		{
			// re-assigning an item from parent container, just move the item to the new location
			To->SwapItems(ExistingItemSlot, ToSlot);
		}
		else
		{
			// assign new item
			To->AddItem(Item, ToSlot);
		}
	}
	else
	{
		// move from another container
		MoveItem(From, To, Item, ToSlot, bAllowPartial);
	}
	return true;
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
	return UGameItemStatics::FindFragment(ItemDef, FragmentClass);
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

UGameItemSubsystem* UGameItemSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World ? UGameInstance::GetSubsystem<UGameItemSubsystem>(World->GetGameInstance()) : nullptr;
}

UGameItemSubsystem* UGameItemSubsystem::GetGameItemSubsystem(const UObject* WorldContextObject)
{
	return Get(WorldContextObject);
}
