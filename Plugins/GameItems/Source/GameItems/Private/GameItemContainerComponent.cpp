// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerComponent.h"

#include "GameItemContainer.h"
#include "GameItemContainerDef.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerComponent)


UGameItemContainerComponent::UGameItemContainerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
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
		CreateDefaultContainers();
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

void UGameItemContainerComponent::CreateDefaultContainers()
{
	check(GetOwner()->HasAuthority());

	for (const auto& DefaultContainer : DefaultContainers)
	{
		CreateContainer(DefaultContainer.Key, DefaultContainer.Value);
	}
}

UGameItemContainer* UGameItemContainerComponent::CreateContainer(FGameplayTag ContainerId, TSubclassOf<UGameItemContainerDef> ContainerDef)
{
	if (!ContainerId.IsValid() || Containers.Contains(ContainerId))
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
	NewContainer->ContainerDef = ContainerDef;

	AddContainer(NewContainer);

	return NewContainer;
}

void UGameItemContainerComponent::AddContainer(UGameItemContainer* Container)
{
	check(Container);
	check(!Containers.Contains(Container->ContainerId));

	Containers.Add(Container->ContainerId, Container);

	AddReplicatedSubObject(Container);

	Container->AddDefaultItems();
}
