// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerComponent.h"

#include "GameItem.h"
#include "GameItemDef.h"
#include "GameItemSubsystem.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"


UGameItemContainerComponent::UGameItemContainerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	ItemList.OnListChangedEvent.AddUObject(this, &UGameItemContainerComponent::OnListChanged);
}

void UGameItemContainerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemList);
}

UGameItem* UGameItemContainerComponent::CreateItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count) const
{
	check(ItemDef != nullptr);

	AActor* OwningActor = GetOwner();
	check(OwningActor->HasAuthority());

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UGameItemSubsystem* ItemSubsystem = World->GetGameInstance()->GetSubsystem<UGameItemSubsystem>();

	return ItemSubsystem->CreateGameItem(OwningActor, ItemDef, Count);
}

bool UGameItemContainerComponent::CanAddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	// TODO: add stack count limits, etc
	return true;
}

UGameItem* UGameItemContainerComponent::AddNewItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count)
{
	if (!ItemDef)
	{
		return nullptr;
	}

	UGameItem* NewItem = CreateItem(ItemDef, Count);
	if (!NewItem)
	{
		return nullptr;
	}

	AddItem(NewItem);

	return NewItem;
}

void UGameItemContainerComponent::AddItem(UGameItem* Item)
{
	if (!Item)
	{
		return;
	}

	ItemList.AddEntry(Item);
	OnItemAdded(Item);
}

void UGameItemContainerComponent::RemoveItem(UGameItem* Item)
{
	if (!Item)
	{
		return;
	}

	ItemList.RemoveEntry(Item);
	OnItemRemoved(Item);
}

TArray<UGameItem*> UGameItemContainerComponent::GetAllItems() const
{
	TArray<UGameItem*> Result;
	ItemList.GetAllItems(Result);
	return Result;
}

UGameItem* UGameItemContainerComponent::FindFirstItemByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item) && Entry.Item->GetItemDef() == ItemDef)
		{
			return Entry.Item;
		}
	}
	return nullptr;
}

int32 UGameItemContainerComponent::GetTotalItemCountByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	int32 Total = 0;
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item) && Entry.Item->GetItemDef() == ItemDef)
		{
			Total += Entry.Item->GetCount();
		}
	}
	return Total;
}

void UGameItemContainerComponent::OnItemAdded(UGameItem* Item)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Item)
	{
		AddReplicatedSubObject(Item);
	}
}

void UGameItemContainerComponent::OnItemRemoved(UGameItem* Item)
{
	if (IsUsingRegisteredSubObjectList() && Item)
	{
		RemoveReplicatedSubObject(Item);
	}
}

void UGameItemContainerComponent::OnListChanged(FGameItemListEntry& Entry, int32 NewCount, int32 OldCount)
{
	// re-broadcast the change from this component
	// TODO
}

void UGameItemContainerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// register any existing items
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FGameItemListEntry& Entry : ItemList.Entries)
		{
			if (IsValid(Entry.Item))
			{
				AddReplicatedSubObject(Entry.Item);
			}
		}
	}
}

bool UGameItemContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bDidWrite = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// replicate all item instances in this container
	for (const FGameItemListEntry& Entry : ItemList.Entries)
	{
		if (IsValid(Entry.Item))
		{
			bDidWrite |= Channel->ReplicateSubobject(Entry.Item, *Bunch, *RepFlags);
		}
	}

	return bDidWrite;
}
