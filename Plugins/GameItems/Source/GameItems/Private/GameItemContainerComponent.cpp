// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerComponent.h"

#include "GameItemContainer.h"
#include "GameItemContainerDef.h"
#include "GameItemContainerGraph.h"
#include "GameItemDef.h"
#include "GameItemSaveDataInterface.h"
#include "GameItemSettings.h"
#include "GameItemsModule.h"
#include "GameItemStatics.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/SaveGame.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
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
	}

	if (bAutoAddDefaultItems)
	{
		CreateDefaultItems();
	}
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
	for (const UGameItemContainer* Container : Containers)
	{
		// only parent containers contribute to collection count
		if (IsValid(Container) && !Container->IsChild())
		{
			Result += Container->GetTotalMatchingItemCount(Item);
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
	for (const UGameItemContainer* Container : Containers)
	{
		// only parent containers contribute to collection count
		if (IsValid(Container) && !Container->IsChild())
		{
			Result += Container->GetTotalItemCountByDef(ItemDef);
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
	for (UGameItemContainer* Container : Containers)
	{
		if (!IsValid(Container) || Container->IsChild())
		{
			continue;
		}

		if (!Container->HasSaveAndLoadAuthority())
		{
			UE_LOG(LogGameItems, Verbose, TEXT("[%hs] Skipping container with no save/load authority: %s"),
				__func__, *Container->GetReadableName());
			continue;
		}

		FGameItemContainerSaveData& ContainerData = CollectionData.Containers.FindOrAdd(Container->GetContainerId());
		Container->CommitSaveData(ContainerData, SavedItems);
	}

	// ...then save all children, now that item guids have been created
	for (UGameItemContainer* Container : Containers)
	{
		if (!IsValid(Container) || !Container->IsChild())
		{
			continue;
		}

		if (!Container->HasSaveAndLoadAuthority())
		{
			UE_LOG(LogGameItems, Verbose, TEXT("[%hs] Skipping container with no save/load authority: %s"),
				__func__, *Container->GetReadableName());
			continue;
		}

		FGameItemContainerSaveData& ContainerData = CollectionData.Containers.FindOrAdd(Container->GetContainerId());
		Container->CommitSaveData(ContainerData, SavedItems);
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
	for (UGameItemContainer* Container : Containers)
	{
		if (!IsValid(Container) || Container->IsChild())
		{
			continue;
		}

		// TODO: validate that there are no issues with local-authority and server-authority containers coexisting
		if (!Container->HasSaveAndLoadAuthority())
		{
			UE_LOG(LogGameItems, Verbose, TEXT("[%hs] Skipping container with no save/load authority: %s"),
				__func__, *Container->GetReadableName());
			continue;
		}

		const FGameItemContainerSaveData ContainerData = CollectionData.Containers.FindRef(Container->GetContainerId());
		Container->LoadSaveData(ContainerData, LoadedItems);
	}

	// ...then load all children, now that items have been created
	for (UGameItemContainer* Container : Containers)
	{
		if (!IsValid(Container) || !Container->IsChild())
		{
			continue;
		}

		if (!Container->HasSaveAndLoadAuthority())
		{
			UE_LOG(LogGameItems, Verbose, TEXT("[%hs] Skipping container with no save/load authority: %s"),
				__func__, *Container->GetReadableName());
			continue;
		}

		const FGameItemContainerSaveData ContainerData = CollectionData.Containers.FindRef(Container->GetContainerId());
		Container->LoadSaveData(ContainerData, LoadedItems);
	}
}

void UGameItemContainerComponent::AddDefaultContainers()
{
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

	if (!ContainerLinks.IsEmpty())
	{
#if !NO_LOGGING
		FMessageLog("PIE").Warning()
		                  ->AddText(INVTEXT("ContainerLinks is deprecated, create a GameItemContainerGraph to setup multiple connected containers"))
		                  ->AddToken(FUObjectToken::Create(GetOwner()->GetClass()));
#endif

		for (const FGameItemContainerLinkSpec& Link : ContainerLinks)
		{
			CreateContainerLink(Link, this, false);
		}
	}

	ResolveAllContainerLinks();
}

void UGameItemContainerComponent::CreateDefaultItems(bool bForce)
{
	for (UGameItemContainer* Container : Containers)
	{
		Container->CreateDefaultItems(bForce);
	}
}

void UGameItemContainerComponent::ResolveAllContainerLinks(bool bForce)
{
	for (const TObjectPtr<UGameItemContainer>& Container : Containers)
	{
		if (!IsValid(Container))
		{
			continue;
		}

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
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}
	if (!Graph)
	{
		UE_LOG(LogGameItems, Error, TEXT("%s AddContainerGraph called with null Graph"),
		       *GetDebugPrefix());
		return;
	}

	if (Graphs.Contains(Graph))
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s Graph already added: %s"),
		       *GetDebugPrefix(), *Graph->GetName());
		return;
	}

	Graphs.Add(Graph);

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s Adding container graph: %s"),
	       *GetDebugPrefix(), *Graph->GetName());

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
}

UGameItemContainer* UGameItemContainerComponent::CreateContainer(const FGameItemContainerSpec& ContainerSpec, bool bResolveLinks)
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return nullptr;
	}
	if (!ContainerSpec.IsValid())
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s Invalid container spec, make sure ContainerDef and ContainerId are set: %s"),
		       *GetDebugPrefix(), *ContainerSpec.DisplayName.ToString());
		return nullptr;
	}

	if (ContainerMap.Contains(ContainerSpec.ContainerId))
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s Container already exists with id: %s"),
		       *GetDebugPrefix(), *ContainerSpec.ContainerId.ToString());
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

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s Created container: %s (%s)"),
	       *GetDebugPrefix(), *ContainerSpec.ContainerId.ToString(), *ContainerSpec.ContainerDef->GetName());

	// add any already-defined links
	AddMatchingLinkRulesToContainer(NewContainer, Links);

	// register container (and broadcast container add events)
	AddContainer(NewContainer);

	if (bResolveLinks)
	{
		ResolveAllContainerLinks();
	}

	return NewContainer;
}

void UGameItemContainerComponent::CreateContainerLink(const FGameItemContainerLinkSpec& LinkSpec, const UObject* SourceObject, bool bResolveLinks)
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	const FActiveGameItemContainerLink& NewLink = Links.Emplace_GetRef(LinkSpec, SourceObject);

	// add new link to existing containers (that match)
	for (const TObjectPtr<UGameItemContainer>& Container : Containers)
	{
		if (IsValid(Container))
		{
			AddLinkRuleToContainer(Container, NewLink);
		}
	}

	if (bResolveLinks)
	{
		ResolveAllContainerLinks();
	}
}

bool UGameItemContainerComponent::ContainsItemInAnyContainer(const UGameItem* Item) const
{
	// reverse look-up by the item's containers
	for (const UGameItemContainer* Container : Item->GetContainers())
	{
		if (Containers.Contains(Container))
		{
			return true;
		}
	}
	return false;
}

void UGameItemContainerComponent::AddContainer(UGameItemContainer* Container)
{
	if (!ensure(GetOwner()->HasAuthority()))
	{
		return;
	}

	check(Container);
	ensureAlways(!Containers.Contains(Container));
	ensureAlways(!ContainerMap.Contains(Container->GetContainerId()));

	Containers.Emplace(Container);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Containers, this);

	ContainerMap.Emplace(Container->GetContainerId(), Container);

	OnContainerAdded(Container);
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
	Container->OnItemAddedEvent.AddUObject(this, &ThisClass::OnItemAddedToContainer, Container);
	Container->OnItemRemovedEvent.AddUObject(this, &ThisClass::OnItemRemovedFromContainer, Container);
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
	check(Container);

	for (const FActiveGameItemContainerLink& Link : InLinks)
	{
		AddLinkRuleToContainer(Container, Link);
	}
}

void UGameItemContainerComponent::AddLinkRuleToContainer(UGameItemContainer* Container, const FActiveGameItemContainerLink& Link)
{
	check(Container);

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

			UE_LOG(LogGameItems, VeryVerbose, TEXT("%s Linked %s -> %s (%s)"),
				*GetDebugPrefix(),
				*Container->GetContainerId().ToString(), *NewLink->LinkedContainerId.ToString(),
				*Link.LinkSpec.ContainerLinkClass->GetName());
		}
	}
}

void UGameItemContainerComponent::OnRep_Containers(const TArray<UGameItemContainer*>& PreviousContainers)
{
	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] Containers: %d"),
		*GetDebugPrefix(), __func__, Containers.Num());

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
		if (IsValid(Container))
		{
			ContainerMap.Emplace(Container->GetContainerId(), Container);
		}
	}
}

void UGameItemContainerComponent::OnItemAddedToContainer(UGameItem* GameItem, UGameItemContainer* Container)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && GameItem)
	{
		AddReplicatedSubObject(GameItem);
	}

	if (!Container->IsChild())
	{
		OnItemAddedEvent.Broadcast(GameItem);
	}
}

void UGameItemContainerComponent::OnItemRemovedFromContainer(UGameItem* GameItem, UGameItemContainer* Container)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && GameItem && GameItem->GetContainers().IsEmpty())
	{
		RemoveReplicatedSubObject(GameItem);
	}

	if (!Container->IsChild())
	{
		// items should never belong to more than one parent container
		if (ensure(!ContainsItemInAnyContainer(GameItem)))
		{
			OnItemRemovedEvent.Broadcast(GameItem);
		}
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

FString UGameItemContainerComponent::GetDebugPrefix() const
{
	return FString::Printf(TEXT("%s[%s]"), *UGameItemStatics::GetNetDebugPrefix(this), *GetReadableName());
}
