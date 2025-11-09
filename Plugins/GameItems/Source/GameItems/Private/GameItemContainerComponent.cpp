// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerComponent.h"

#include "GameItemContainer.h"
#include "GameItemContainerDef.h"
#include "GameItemDef.h"
#include "GameItemSaveDataInterface.h"
#include "GameItemSettings.h"
#include "GameItemsModule.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/SaveGame.h"
#include "Net/UnrealNetwork.h"
#include "Rules/GameItemContainerLink.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerComponent)


UGameItemContainerComponent::UGameItemContainerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);

	FGameItemContainerSpec& DefaultContainerSpec = StartupContainers.AddDefaulted_GetRef();
	DefaultContainerSpec.ContainerId = UGameItemSettings::GetDefaultContainerId();
}

void UGameItemContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerComponent, Containers, SharedParams);
}

void UGameItemContainerComponent::InitializeComponent()
{
	Super::InitializeComponent();

#if WITH_SERVER_CODE
	const UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld())
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		CreateStartupContainers();
		CreateDefaultItems();
	}
#endif
}

void UGameItemContainerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// register any existing items
	if (IsUsingRegisteredSubObjectList())
	{
		for (const TPair<FGameplayTag, UGameItemContainer*>& Elem : ContainerMap)
		{
			if (IsValid(Elem.Value))
			{
				AddReplicatedSubObject(Elem.Value);
			}
		}
	}
}

bool UGameItemContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bDidWrite = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (const TPair<FGameplayTag, UGameItemContainer*>& Elem : ContainerMap)
	{
		if (IsValid(Elem.Value))
		{
			bDidWrite |= Channel->ReplicateSubobject(Elem.Value, *Bunch, *RepFlags);
		}
	}
	return bDidWrite;
}

TArray<UGameItemContainer*> UGameItemContainerComponent::GetAllItemContainers() const
{
	return Containers;
}

UGameItemContainer* UGameItemContainerComponent::GetItemContainer(FGameplayTag ContainerId) const
{
	return ContainerMap.FindRef(ContainerId);
}

int32 UGameItemContainerComponent::GetTotalMatchingItemCount(const UGameItem* Item) const
{
	if (!Item)
	{
		return 0;
	}

	// TODO: cache the counts for faster lookup
	int32 Result = 0;
	for (const auto& Elem : ContainerMap)
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
	for (const auto& Elem : ContainerMap)
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
	for (const auto& ContainerElem : ContainerMap)
	{
		UGameItemContainer* Container = ContainerElem.Value;
		if (!Container->IsChild())
		{
			FGameItemContainerSaveData& ContainerData = CollectionData.Containers.FindOrAdd(Container->ContainerId);
			Container->CommitSaveData(ContainerData, SavedItems);
		}
	}

	// ...then save all children, now that item guids have been created
	for (const auto& ContainerElem : ContainerMap)
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
	for (const auto& ContainerElem : ContainerMap)
	{
		UGameItemContainer* Container = ContainerElem.Value;
		if (!Container->IsChild())
		{
			const FGameItemContainerSaveData ContainerData = CollectionData.Containers.FindRef(Container->ContainerId);
			Container->LoadSaveData(ContainerData, LoadedItems);
		}
	}

	// ...then load all children, now that items have been created
	for (const auto& ContainerElem : ContainerMap)
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
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("[%s] Creating startup containers..."), *GetReadableName());
	for (const FGameItemContainerSpec& ContainerSpec : StartupContainers)
	{
		if (UGameItemContainer* Container = CreateContainer(ContainerSpec.ContainerId, ContainerSpec.ContainerDef))
		{
			Container->DisplayName = ContainerSpec.DisplayName;
		}
	}
#endif
}

void UGameItemContainerComponent::CreateDefaultItems(bool bForce)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("[%s] Creating default items..."), *GetReadableName());
	for (auto& Elem : ContainerMap)
	{
		Elem.Value->CreateDefaultItems(bForce);
	}
#endif
}

void UGameItemContainerComponent::ResolveContainerLinks()
{
	for (const auto& Elem : ContainerMap)
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
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return nullptr;
	}

	if (ContainerMap.Contains(ContainerId))
	{
		// already exists, or invalid id
		UE_LOG(LogGameItems, Warning, TEXT("[%s] Container already exists with id: %s"),
		       *GetReadableName(), *ContainerId.ToString());
		return nullptr;
	}

	if (!ContainerDef)
	{
		// must provide a container def
		UE_LOG(LogGameItems, Warning, TEXT("[%s] Attempted to create container with null ContainerDef: %s"),
		       *GetReadableName(), *ContainerId.ToString());
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

	UE_LOG(LogGameItems, VeryVerbose, TEXT("[%s] Created container: %s (%s)"),
	       *GetReadableName(), *ContainerId.ToString(), *ContainerDef->GetName());

	// add link rules
	for (const FGameItemContainerLinkSpec& LinkSpec : ContainerLinks)
	{
		if (!LinkSpec.ContainerLinkClass)
		{
			continue;
		}
		if (LinkSpec.ContainerQuery.Matches(NewContainer->GetOwnedTags()))
		{
			if (UGameItemContainerLink* NewLink = NewContainer->AddRule<UGameItemContainerLink>(LinkSpec.ContainerLinkClass))
			{
				// just set the container id, then resolve this (and any other links) later
				NewLink->LinkedContainerId = LinkSpec.LinkedContainerId;
				UE_LOG(LogGameItems, VeryVerbose, TEXT("[%s] Linked %s to %s (%s)"),
				       *GetReadableName(), *ContainerId.ToString(), *LinkSpec.LinkedContainerId.ToString(), *LinkSpec.ContainerLinkClass->GetName());
			}
		}
	}

	AddContainer(NewContainer);

	ResolveContainerLinks();

	return NewContainer;
#else
	return nullptr;
#endif
}

void UGameItemContainerComponent::AddContainer(UGameItemContainer* Container)
{
	check(Container);
	check(!ContainerMap.Contains(Container->ContainerId));

	Containers.Emplace(Container);
	ContainerMap.Emplace(Container->ContainerId, Container);

	// containers can never be removed (yet), so there is no matching RemoveReplicatedSubObject
	AddReplicatedSubObject(Container);

	// monitor for items added/removed so we can replicate those too
	Container->OnItemAddedEvent.AddUObject(this, &ThisClass::OnItemAdded);
	Container->OnItemRemovedEvent.AddUObject(this, &ThisClass::OnItemRemoved);
}

void UGameItemContainerComponent::OnRep_Containers()
{
	// update container map
	ContainerMap.Reset();
	for (const TObjectPtr<UGameItemContainer>& Container : Containers)
	{
		ContainerMap.Emplace(Container->ContainerId, Container);
	}
}

void UGameItemContainerComponent::OnItemAdded(UGameItem* GameItem)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && GameItem)
	{
		AddReplicatedSubObject(GameItem);
	}
}

void UGameItemContainerComponent::OnItemRemoved(UGameItem* GameItem)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && GameItem)
	{
		RemoveReplicatedSubObject(GameItem);
	}
}
