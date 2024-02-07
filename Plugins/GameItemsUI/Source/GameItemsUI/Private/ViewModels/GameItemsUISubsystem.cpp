// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/GameItemsUISubsystem.h"

#include "GameItemContainer.h"
#include "GameItemSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ViewModels/GameItemContainerViewModel.h"


UGameItemContainerViewModel* UGameItemsUISubsystem::GetOrCreateContainerViewModel(UGameItemContainer* Container)
{
	if (!Container)
	{
		return nullptr;
	}

	UGameItemContainerViewModel** ContainerViewModel = ContainerViewModels.FindByPredicate([Container](UGameItemContainerViewModel* ViewModel)
	{
		return ViewModel && ViewModel->GetContainer() == Container;
	});

	if (ContainerViewModel)
	{
		return *ContainerViewModel;
	}

	// create a new view model
	UGameItemContainerViewModel* NewViewModel = CreateContainerViewModel(Container);
	check(NewViewModel);
	ContainerViewModels.Add(NewViewModel);
	return NewViewModel;
}

UGameItemContainerViewModel* UGameItemsUISubsystem::GetOrCreateContainerViewModelForActor(AActor* Actor, FGameplayTag ContainerId)
{
	const UGameItemSubsystem* ItemSubsystem = UGameInstance::GetSubsystem<UGameItemSubsystem>(GetWorld()->GetGameInstance());
	UGameItemContainer* Container = ItemSubsystem->GetContainerForActor(Actor, ContainerId);
	return GetOrCreateContainerViewModel(Container);
}

UGameItemContainerViewModel* UGameItemsUISubsystem::CreateContainerViewModel(UGameItemContainer* Container)
{
	check(Container);
	UGameItemContainerViewModel* NewViewModel = NewObject<UGameItemContainerViewModel>(this);
	NewViewModel->SetContainer(Container);
	return NewViewModel;
}
