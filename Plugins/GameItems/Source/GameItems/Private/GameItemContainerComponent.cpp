// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerComponent.h"

#include "GameItemContainer.h"
#include "GameItemContainerDef.h"
#include "GameItemDef.h"
#include "GameItemSaveDataInterface.h"
#include "GameItemSettings.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/SaveGame.h"
#include "Rules/GameItemContainerLink.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerComponent)


UGameItemContainerComponent::UGameItemContainerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  bEnableSaveGame(false),
	  SaveCollectionId(NAME_None),
	  bIsPlayerCollection(false)
{
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);

	FGameItemContainerSpec& DefaultContainerSpec = StartupContainers.AddDefaulted_GetRef();
	DefaultContainerSpec.ContainerId = UGameItemSettings::GetDefaultContainerId();
}

void UGameItemContainerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	const UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		CreateStartupContainers();
	}
}

void UGameItemContainerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// TODO: register any existing containers...
	// register any existing items
	// if (IsUsingRegisteredSubObjectList())
	// {
	// 	for (const FGameItemListEntry& Entry : ItemList.Entries)
	// 	{
	// 		if (IsValid(Entry.GetItem()))
	// 		{
	// 			AddReplicatedSubObject(Entry.GetItem());
	// 		}
	// 	}
	// }
}

bool UGameItemContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bDidWrite = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// TODO: replicate all containers...
	// replicate all item instances in this container
	// for (const FGameItemListEntry& Entry : ItemList.Entries)
	// {
	// 	if (IsValid(Entry.GetItem()))
	// 	{
	// 		bDidWrite |= Channel->ReplicateSubobject(Entry.GetItem(), *Bunch, *RepFlags);
	// 	}
	// }

	return bDidWrite;
}

TArray<UGameItemContainer*> UGameItemContainerComponent::GetAllItemContainers() const
{
	TArray<UGameItemContainer*> AllContainers;
	Containers.GenerateValueArray(AllContainers);
	return AllContainers;
}

UGameItemContainer* UGameItemContainerComponent::GetItemContainer(FGameplayTag ContainerId) const
{
	return Containers.FindRef(ContainerId);
}

int32 UGameItemContainerComponent::GetTotalMatchingItemCount(const UGameItem* Item) const
{
	if (!Item)
	{
		return 0;
	}

	// TODO: cache the counts for faster lookup
	int32 Result = 0;
	for (const auto& Elem : Containers)
	{
		// only parent containers contribute to collection count
		if (!Elem.Value->IsChild())
		{
			Result += Elem.Value->GetTotalMatchingItemCount(Item);
		}
	}
	return Result;
}

int32 UGameItemContainerComponent::GetTotalMatchingItemCountByDef(TSubclassOf<UGameItemDef> ItemDef) const
{
	if (!ItemDef)
	{
		return 0;
	}

	// TODO: cache the counts for faster lookup
	int32 Result = 0;
	for (const auto& Elem : Containers)
	{
		// only parent containers contribute to collection count
		if (!Elem.Value->IsChild())
		{
			Result += Elem.Value->GetTotalItemCountByDef(ItemDef);
		}
	}
	return Result;
}

void UGameItemContainerComponent::CommitSaveGame(USaveGame* SaveGame)
{
	IGameItemSaveDataInterface* ItemSaveDataInterface = Cast<IGameItemSaveDataInterface>(SaveGame);
	if (!ItemSaveDataInterface)
	{
		return;
	}

	FPlayerAndWorldGameItemSaveData& AllSaveData = ItemSaveDataInterface->GetItemSaveData();
	FGameItemContainerCollectionSaveData& CollectionData = bIsPlayerCollection
		                                                       ? AllSaveData.PlayerItemData.FindOrAdd(SaveCollectionId)
		                                                       : AllSaveData.WorldItemData.FindOrAdd(SaveCollectionId);

	TMap<UGameItem*, FGuid> SavedItems;

	// save parent containers
	for (const auto& ContainerElem : Containers)
	{
		UGameItemContainer* Container = ContainerElem.Value;
		if (!Container->IsChild())
		{
			FGameItemContainerSaveData& ContainerData = CollectionData.Containers.FindOrAdd(Container->ContainerId);
			Container->CommitSaveData(ContainerData, SavedItems);
		}
	}

	// ...then save all children, now that item guids have been created
	for (const auto& ContainerElem : Containers)
	{
		UGameItemContainer* Container = ContainerElem.Value;
		if (Container->IsChild())
		{
			FGameItemContainerSaveData& ContainerData = CollectionData.Containers.FindOrAdd(Container->ContainerId);
			Container->CommitSaveData(ContainerData, SavedItems);
		}
	}
}

void UGameItemContainerComponent::LoadSaveGame(USaveGame* SaveGame)
{
	IGameItemSaveDataInterface* ItemSaveDataInterface = Cast<IGameItemSaveDataInterface>(SaveGame);
	if (!ItemSaveDataInterface)
	{
		return;
	}

	FPlayerAndWorldGameItemSaveData& AllSaveData = ItemSaveDataInterface->GetItemSaveData();
	const FGameItemContainerCollectionSaveData& CollectionData = bIsPlayerCollection
		                                                             ? AllSaveData.PlayerItemData.FindOrAdd(SaveCollectionId)
		                                                             : AllSaveData.WorldItemData.FindOrAdd(SaveCollectionId);

	TMap<FGuid, UGameItem*> LoadedItems;

	// load parent containers
	for (const auto& ContainerElem : Containers)
	{
		UGameItemContainer* Container = ContainerElem.Value;
		if (!Container->IsChild())
		{
			const FGameItemContainerSaveData ContainerData = CollectionData.Containers.FindRef(Container->ContainerId);
			Container->LoadSaveData(ContainerData, LoadedItems);
		}
	}

	// ...then load all children, now that items have been created
	for (const auto& ContainerElem : Containers)
	{
		UGameItemContainer* Container = ContainerElem.Value;
		if (Container->IsChild())
		{
			const FGameItemContainerSaveData ContainerData = CollectionData.Containers.FindRef(Container->ContainerId);
			Container->LoadSaveData(ContainerData, LoadedItems);
		}
	}
}

void UGameItemContainerComponent::CreateStartupContainers()
{
	check(GetOwner()->HasAuthority());

	for (const FGameItemContainerSpec& ContainerSpec : StartupContainers)
	{
		UGameItemContainer* Container = CreateContainer(ContainerSpec.ContainerId, ContainerSpec.ContainerDef);
		if (Container)
		{
			Container->DisplayName = ContainerSpec.DisplayName;
		}
	}
}

void UGameItemContainerComponent::ResolveContainerLinks()
{
	for (const auto& Elem : Containers)
	{
		const UGameItemContainer* Container = Elem.Value;
		const TArray<UGameItemContainerRule*>& Rules = Container->GetRules();
		for (UGameItemContainerRule* Rule : Rules)
		{
			if (UGameItemContainerLink* LinkRule = Cast<UGameItemContainerLink>(Rule))
			{
				if (!LinkRule->GetLinkedContainer() && LinkRule->LinkedContainerId.IsValid())
				{
					LinkRule->SetLinkedContainer(GetItemContainer(LinkRule->LinkedContainerId));
				}
			}
		}
	}
}

bool UGameItemContainerComponent::IsItemSlotted(UGameItem* Item, FGameplayTagContainer ContainerTags) const
{
	const TArray<UGameItemContainer*> AllContainers = GetAllItemContainers();
	for (const UGameItemContainer* Container : AllContainers)
	{
		if (Container->GetOwnedTags().HasAny(ContainerTags))
		{
			if (Container->Contains(Item))
			{
				return true;
			}
		}
	}
	return false;
}

UGameItemContainer* UGameItemContainerComponent::CreateContainer(FGameplayTag ContainerId, TSubclassOf<UGameItemContainerDef> ContainerDef)
{
	if (Containers.Contains(ContainerId))
	{
		// already exists, or invalid id
		return nullptr;
	}

	if (!ContainerDef)
	{
		// must provide a container def
		return nullptr;
	}

	// retrieve container class to spawn from the definition
	const UGameItemContainerDef* DefCDO = GetDefault<UGameItemContainerDef>(ContainerDef);
	TSubclassOf<UGameItemContainer> ContainerClass = DefCDO->ContainerClass;
	if (!ContainerClass)
	{
		ContainerClass = UGameItemContainer::StaticClass();
	}

	// create and initialize the new container
	UGameItemContainer* NewContainer = NewObject<UGameItemContainer>(this, ContainerClass);
	check(NewContainer);
	NewContainer->ContainerId = ContainerId;
	NewContainer->SetCollection(this);
	NewContainer->SetContainerDef(ContainerDef);

	// add link rules
	for (const FGameItemContainerLinkSpec& LinkSpec : ContainerLinks)
	{
		if (!LinkSpec.ContainerLinkClass)
		{
			continue;
		}
		if (LinkSpec.ContainerQuery.Matches(NewContainer->GetOwnedTags()))
		{
			UGameItemContainerLink* NewLink = NewContainer->AddRule<UGameItemContainerLink>(LinkSpec.ContainerLinkClass);
			if (NewLink)
			{
				// just set the container id, then resolve this (and any other links) later
				NewLink->LinkedContainerId = LinkSpec.LinkedContainerId;
			}
		}
	}

	AddContainer(NewContainer);

	ResolveContainerLinks();

	return NewContainer;
}

void UGameItemContainerComponent::AddContainer(UGameItemContainer* Container)
{
	check(Container);
	check(!Containers.Contains(Container->ContainerId));

	Containers.Add(Container->ContainerId, Container);

	AddReplicatedSubObject(Container);

	// add default items after loading save game and gameplay begins
	Container->AddDefaultItems();
}
