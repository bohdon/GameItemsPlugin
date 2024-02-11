// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerComponent.h"

#include "GameItemAutoSlotRule.h"
#include "GameItemContainer.h"
#include "GameItemContainerDef.h"
#include "GameItemContainerLink.h"
#include "GameItemSettings.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerComponent)


UGameItemContainerComponent::UGameItemContainerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
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

TArray<UGameItem*> UGameItemContainerComponent::TryAutoSlotItem(UGameItem* Item, FGameplayTagContainer ContextTags) const
{
	const TArray<UGameItemContainer*> AllContainers = GetAllItemContainers();

	for (const UGameItemAutoSlotRule* AutoSlotRule : AutoSlotRules)
	{
		if (!AutoSlotRule)
		{
			continue;
		}

		TArray<UGameItem*> Result = AutoSlotRule->TryAutoSlotItem(Item, AllContainers, ContextTags);
		if (!Result.IsEmpty())
		{
			return Result;
		}
	}
	return TArray<UGameItem*>();
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

void UGameItemContainerComponent::PostLoad()
{
	Super::PostLoad();

	// cleanup empty rules
	AutoSlotRules.RemoveAll([](const UGameItemAutoSlotRule* Rule)
	{
		return Rule == nullptr;
	});
}

void UGameItemContainerComponent::AddContainer(UGameItemContainer* Container)
{
	check(Container);
	check(!Containers.Contains(Container->ContainerId));

	Containers.Add(Container->ContainerId, Container);

	AddReplicatedSubObject(Container);

	Container->AddDefaultItems();
}
