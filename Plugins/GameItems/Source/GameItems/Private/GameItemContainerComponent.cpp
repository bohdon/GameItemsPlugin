// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerComponent.h"

#include "GameItemContainer.h"
#include "GameItemContainerDef.h"
#include "GameItemContainerGraph.h"
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
}

void UGameItemContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerComponent, Containers, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerComponent, Links, Params);
}

void UGameItemContainerComponent::PostLoad()
{
	Super::PostLoad();

#if !NO_LOGGING
	if (!ContainerLinks.IsEmpty())
	{
		UE_LOG(LogGameItems, Error, TEXT("[%s] ContainerLinks set on the component are deprecated, use DefaultContainerGraphs instead"),
		       *GetReadableName());
	}
#endif
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
		if (bAutoAddDefaultContainers)
		{
			AddDefaultContainers();
		}
		if (bAutoAddDefaultItems)
		{
			CreateDefaultItems();
		}
	}
#endif
}

void UGameItemContainerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// register any existing items
	if (IsUsingRegisteredSubObjectList())
	{
		for (UGameItemContainer* Container : Containers)
		{
			if (IsValid(Container))
			{
				AddReplicatedSubObject(Container);

				for (UGameItemContainerRule* Rule : Container->GetRules())
				{
					if (IsValid(Rule))
					{
						AddReplicatedSubObject(Rule);
					}
				}
			}
		}
	}
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
			FGameItemContainerSaveData& ContainerData = CollectionData.Containers.FindOrAdd(Container->GetContainerId());
			Container->CommitSaveData(ContainerData, SavedItems);
		}
	}

	// ...then save all children, now that item guids have been created
	for (const auto& ContainerElem : ContainerMap)
	{
		UGameItemContainer* Container = ContainerElem.Value;
		if (Container->IsChild())
		{
			FGameItemContainerSaveData& ContainerData = CollectionData.Containers.FindOrAdd(Container->GetContainerId());
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
			const FGameItemContainerSaveData ContainerData = CollectionData.Containers.FindRef(Container->GetContainerId());
			Container->LoadSaveData(ContainerData, LoadedItems);
		}
	}

	// ...then load all children, now that items have been created
	for (const auto& ContainerElem : ContainerMap)
	{
		UGameItemContainer* Container = ContainerElem.Value;
		if (Container->IsChild())
		{
			const FGameItemContainerSaveData ContainerData = CollectionData.Containers.FindRef(Container->GetContainerId());
			Container->LoadSaveData(ContainerData, LoadedItems);
		}
	}
}

void UGameItemContainerComponent::AddDefaultContainers()
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	for (const UGameItemContainerGraph* Graph : ObjectPtrDecay(DefaultContainerGraphs))
	{
		if (Graph)
		{
			AddContainerGraph(Graph, false);
		}
	}
	
	for (const FGameItemContainerSpec& ContainerSpec : DefaultContainers)
	{
		CreateContainer(ContainerSpec, false);
	}

	ResolveAllContainerLinks();
#endif
}

void UGameItemContainerComponent::CreateDefaultItems(bool bForce)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] Creating default items..."),
	       *GetNetDebugString(), *GetReadableName());

	for (auto& Elem : ContainerMap)
	{
		Elem.Value->CreateDefaultItems(bForce);
	}
#endif
}

void UGameItemContainerComponent::ResolveAllContainerLinks(bool bForce)
{
	for (const TObjectPtr<UGameItemContainer>& Container : Containers)
	{
		for (UGameItemContainerRule* Rule : Container->GetRules())
		{
			if (UGameItemContainerLink* LinkRule = Cast<UGameItemContainerLink>(Rule))
			{
				LinkRule->ResolveLinkedContainer(this, bForce);
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

void UGameItemContainerComponent::AddContainerGraph(const UGameItemContainerGraph* Graph, bool bResolveLinks)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}
	if (!Graph)
	{
		UE_LOG(LogGameItems, Error, TEXT("%s[%s] AddContainerGraph called with null Graph"),
		       *GetNetDebugString(), *GetReadableName());
		return;
	}

	if (Graphs.Contains(Graph))
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s[%s] Graph already added: %s"),
		       *GetNetDebugString(), *GetReadableName(), *Graph->GetName());
		return;
	}

	Graphs.Add(Graph);

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] Adding container graph: %s"),
	       *GetNetDebugString(), *GetReadableName(), *Graph->GetName());

	// setup links first, so they exist for container add events
	for (const FGameItemContainerLinkSpec& LinkSpec : Graph->Links)
	{
		CreateContainerLink(LinkSpec, Graph, false);
	}

	// create containers
	for (const FGameItemContainerSpec& ContainerSpec : Graph->Containers)
	{
		CreateContainer(ContainerSpec, false);
	}

	if (bResolveLinks)
	{
		ResolveAllContainerLinks();
	}
#endif
}

UGameItemContainer* UGameItemContainerComponent::CreateContainer(const FGameItemContainerSpec& ContainerSpec, bool bResolveLinks)
{
#if WITH_SERVER_CODE
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return nullptr;
	}
	if (!ContainerSpec.IsValid())
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s[%s] Invalid container spec, make sure ContainerDef and ContainerId are set: %s"),
		       *GetNetDebugString(), *GetReadableName(), *ContainerSpec.DisplayName.ToString());
		return nullptr;
	}

	if (ContainerMap.Contains(ContainerSpec.ContainerId))
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s[%s] Container already exists with id: %s"),
		       *GetNetDebugString(), *GetReadableName(), *ContainerSpec.ContainerId.ToString());
		return nullptr;
	}

	// retrieve container class to spawn from the definition
	const UGameItemContainerDef* DefCDO = GetDefault<UGameItemContainerDef>(ContainerSpec.ContainerDef);
	TSubclassOf<UGameItemContainer> ContainerClass = DefCDO->ContainerClass;
	if (!ContainerClass)
	{
		ContainerClass = UGameItemContainer::StaticClass();
	}

	// create and initialize the new container
	UGameItemContainer* NewContainer = NewObject<UGameItemContainer>(this, ContainerClass);
	check(NewContainer);
	NewContainer->SetContainerId(ContainerSpec.ContainerId);
	NewContainer->SetCollection(this);
	NewContainer->SetContainerDef(ContainerSpec.ContainerDef);
	NewContainer->DisplayName = ContainerSpec.DisplayName;

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] Created container: %s (%s)"),
	       *GetNetDebugString(), *GetReadableName(), *ContainerSpec.ContainerId.ToString(), *ContainerSpec.ContainerDef->GetName());

	// add any already-defined links
	AddMatchingLinkRulesToContainer(NewContainer, Links);

	// register container (and broadcast container add events)
	AddContainer(NewContainer);

	if (bResolveLinks)
	{
		ResolveAllContainerLinks();
	}

	return NewContainer;
#else
	return nullptr;
#endif
}

void UGameItemContainerComponent::CreateContainerLink(const FGameItemContainerLinkSpec& LinkSpec, const UObject* SourceObject, bool bResolveLinks)
{
	const FActiveGameItemContainerLink& NewLink = Links.Emplace_GetRef(LinkSpec, SourceObject);

	// add new link to existing containers (that match)
	for (const TObjectPtr<UGameItemContainer>& Container : Containers)
	{
		AddLinkRuleToContainer(Container, NewLink);
	}

	if (bResolveLinks)
	{
		ResolveAllContainerLinks();
	}
}

void UGameItemContainerComponent::AddContainer(UGameItemContainer* Container)
{
#if WITH_SERVER_CODE
	check(Container);
	ensureAlways(!ContainerMap.Contains(Container->GetContainerId()));

	Containers.Emplace(Container);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Containers, this);

	ContainerMap.Emplace(Container->GetContainerId(), Container);

	OnContainerAdded(Container);
#endif
}

void UGameItemContainerComponent::OnContainerAdded(UGameItemContainer* Container)
{
	check(Container);

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(Container);

		// also replicate rules (new rules added later will be replicated in OnRuleAdded below)
		for (UGameItemContainerRule* Rule : Container->GetRules())
		{
			if (IsValid(Rule))
			{
				AddReplicatedSubObject(Rule);
			}
		}
	}

	// monitor for item and rule changes so that all subobjects can be replicated
	Container->OnItemAddedEvent.AddUObject(this, &ThisClass::OnItemAdded);
	Container->OnItemRemovedEvent.AddUObject(this, &ThisClass::OnItemRemoved);
	Container->OnRuleAddedEvent.AddUObject(this, &ThisClass::OnRuleAdded);
	Container->OnRuleRemovedEvent.AddUObject(this, &ThisClass::OnRuleRemoved);

	OnContainerAddedEvent.Broadcast(Container);
}

void UGameItemContainerComponent::OnContainerRemoved(UGameItemContainer* Container)
{
	check(Container);

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		RemoveReplicatedSubObject(Container);

		for (UGameItemContainerRule* Rule : Container->GetRules())
		{
			if (IsValid(Rule))
			{
				RemoveReplicatedSubObject(Rule);
			}
		}
	}

	Container->OnItemAddedEvent.RemoveAll(this);
	Container->OnItemRemovedEvent.RemoveAll(this);
	Container->OnRuleAddedEvent.RemoveAll(this);
	Container->OnRuleRemovedEvent.RemoveAll(this);

	OnContainerRemovedEvent.Broadcast(Container);
}

void UGameItemContainerComponent::AddMatchingLinkRulesToContainer(UGameItemContainer* Container, const TArray<FActiveGameItemContainerLink>& InLinks)
{
#if WITH_SERVER_CODE
	for (const FActiveGameItemContainerLink& Link : InLinks)
	{
		AddLinkRuleToContainer(Container, Link);
	}
#endif
}

void UGameItemContainerComponent::AddLinkRuleToContainer(UGameItemContainer* Container, const FActiveGameItemContainerLink& Link)
{
#if WITH_SERVER_CODE
	if (!Link.LinkSpec.IsValid())
	{
		return;
	}

	if (Link.LinkSpec.ContainerQuery.Matches(Container->GetOwnedTags()))
	{
		if (UGameItemContainerLink* NewLink = Container->AddRule<UGameItemContainerLink>(Link.LinkSpec.ContainerLinkClass))
		{
			// set the id, but don't try to resolve containers yet, since they may not all exist yet
			NewLink->LinkedContainerId = Link.LinkSpec.LinkedContainerId;

			UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] Linked %s to %s (%s)"),
			       *GetNetDebugString(), *GetReadableName(),
			       *Container->GetContainerId().ToString(), *NewLink->LinkedContainerId.ToString(), *Link.LinkSpec.ContainerLinkClass->GetName());
		}
	}
#endif
}

void UGameItemContainerComponent::OnRep_Containers(const TArray<UGameItemContainer*>& PreviousContainers)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s[%s] [%hs] Containers: %d"),
	       *GetNetDebugString(), *GetReadableName(), __func__, Containers.Num());

	// find containers that got removed
	for (UGameItemContainer* PreviousContainer : PreviousContainers)
	{
		if (IsValid(PreviousContainer))
		{
			if (!Containers.Contains(PreviousContainer))
			{
				OnContainerRemoved(PreviousContainer);
			}
		}
	}

	// find containers that got added
	for (UGameItemContainer* NewContainer : Containers)
	{
		if (IsValid(NewContainer))
		{
			if (!PreviousContainers.Contains(NewContainer))
			{
				OnContainerAdded(NewContainer);
			}
		}
	}

	// try to resolve container links
	ResolveAllContainerLinks();

	// update container lookup map
	ContainerMap.Reset();
	for (const TObjectPtr<UGameItemContainer>& Container : Containers)
	{
		ContainerMap.Emplace(Container->GetContainerId(), Container);
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
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && GameItem && GameItem->GetContainers().IsEmpty())
	{
		RemoveReplicatedSubObject(GameItem);
	}
}

void UGameItemContainerComponent::OnRuleAdded(UGameItemContainerRule* Rule)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(Rule);
	}

	// TODO: try to resolve container links? but only on rep?
	// ResolveAllContainerLinks();
}

void UGameItemContainerComponent::OnRuleRemoved(UGameItemContainerRule* Rule)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		RemoveReplicatedSubObject(Rule);
	}
}

FString UGameItemContainerComponent::GetNetDebugString() const
{
	if (ensure(GetWorld()))
	{
		switch (GetWorld()->GetNetMode())
		{
		case NM_DedicatedServer:
		case NM_ListenServer:
			return TEXT("Server: ");
		case NM_Client:
			return FString::Printf(TEXT("Client %d: "), UE::GetPlayInEditorID());
		case NM_Standalone:
		default: ;
		}
	}
	return FString();
}
