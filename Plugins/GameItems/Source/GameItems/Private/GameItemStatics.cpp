// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemStatics.h"

#include "GameItemContainer.h"
#include "GameItemSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"


UGameItemSubsystem* UGameItemStatics::GetItemSubsystemFromContextObject(const UObject* Object)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(Object, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetGameInstance()->GetSubsystem<UGameItemSubsystem>();
	}
	return nullptr;
}

TArray<UGameItemContainer*> UGameItemStatics::GetAllItemContainersForActor(AActor* Actor)
{
	const UGameItemSubsystem* ItemSubsystem = GetItemSubsystemFromContextObject(Actor);
	return ItemSubsystem ? ItemSubsystem->GetAllContainersForActor(Actor) : TArray<UGameItemContainer*>();
}

UGameItemContainer* UGameItemStatics::GetItemContainerForActor(AActor* Actor, FGameplayTag ContainerId)
{
	const UGameItemSubsystem* ItemSubsystem = GetItemSubsystemFromContextObject(Actor);
	return ItemSubsystem ? ItemSubsystem->GetContainerForActor(Actor, ContainerId) : nullptr;
}

const UGameItemFragment* UGameItemStatics::FindGameItemFragment(const UObject* WorldContextObject, TSubclassOf<UGameItemDef> ItemDef,
                                                                TSubclassOf<UGameItemFragment> FragmentClass)
{
	const UGameItemSubsystem* ItemSubsystem = GetItemSubsystemFromContextObject(WorldContextObject);
	return ItemSubsystem ? ItemSubsystem->FindFragment(ItemDef, FragmentClass) : nullptr;
}

UGameItemContainer* UGameItemStatics::GetItemContainerById(const TArray<UGameItemContainer*>& Containers, FGameplayTag ContainerId)
{
	for (UGameItemContainer* Container : Containers)
	{
		if (Container->ContainerId == ContainerId)
		{
			return Container;
		}
	}
	return nullptr;
}
