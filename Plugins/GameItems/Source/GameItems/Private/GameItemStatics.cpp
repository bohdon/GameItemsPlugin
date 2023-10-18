// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemStatics.h"

#include "GameItemContainerComponent.h"
#include "GameItemContainerInterface.h"
#include "GameItemSubsystem.h"


UGameItemSubsystem* UGameItemStatics::GetItemSubsystemFromContextObject(const UObject* Object)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(Object, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetGameInstance()->GetSubsystem<UGameItemSubsystem>();
	}
	return nullptr;
}

TArray<UGameItemContainerComponent*> UGameItemStatics::GetAllGameItemContainersFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return TArray<UGameItemContainerComponent*>();
	}

	// try using interface
	if (const IGameItemContainerInterface* ContainerInterface = Cast<IGameItemContainerInterface>(Actor))
	{
		return ContainerInterface->GetAllItemContainerComponent();
	}

	// fallback to getting components directly
	TArray<UGameItemContainerComponent*> ItemContainers;
	Actor->GetComponents<UGameItemContainerComponent>(ItemContainers);
	return ItemContainers;
}

UGameItemContainerComponent* UGameItemStatics::GetGameItemContainerFromActor(AActor* Actor, FGameplayTag IdTag)
{
	if (!Actor)
	{
		return nullptr;
	}

	// try using interface
	if (const IGameItemContainerInterface* ContainerInterface = Cast<IGameItemContainerInterface>(Actor))
	{
		return ContainerInterface->GetItemContainerComponent(IdTag);
	}

	// fallback to getting components directly
	TInlineComponentArray<UGameItemContainerComponent*> ItemContainers(Actor);
	for (UGameItemContainerComponent* ItemContainer : ItemContainers)
	{
		if (ItemContainer->IdTag == IdTag)
		{
			return ItemContainer;
		}
	}

	return nullptr;
}

const UGameItemFragment* UGameItemStatics::FindGameItemFragment(const UObject* WorldContextObject, TSubclassOf<UGameItemDef> ItemDef,
                                                                TSubclassOf<UGameItemFragment> FragmentClass)
{
	const UGameItemSubsystem* ItemSubsystem = GetItemSubsystemFromContextObject(WorldContextObject);
	if (!ItemSubsystem)
	{
		return nullptr;
	}

	return ItemSubsystem->FindFragment(ItemDef, FragmentClass);
}
