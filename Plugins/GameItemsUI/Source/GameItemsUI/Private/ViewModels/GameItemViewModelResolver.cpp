// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels\GameItemViewModelResolver.h"

#include "GameItemContainerProvider.h"
#include "GameItemSettings.h"
#include "GameItemsUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "ViewModels/GameItemContainerViewModel.h"
#include "ViewModels/GameItemSlotViewModel.h"


// UGameItemViewModelResolverBase
// ------------------------------

UGameItemContainerViewModel* UGameItemViewModelResolverBase::GetItemContainerViewModel(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	// get container from provider
	UGameItemContainer* Container = GetItemContainer(UserWidget, View);
	if (!Container)
	{
		return nullptr;
	}

	// get view model for the container
	UGameItemsUISubsystem* GameItemsUISubsystem = UserWidget->GetWorld()->GetSubsystem<UGameItemsUISubsystem>();
	return GameItemsUISubsystem->GetOrCreateContainerViewModel(Container);
}

UGameItemContainer* UGameItemViewModelResolverBase::GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	return nullptr;
}


// UGameItemContainerViewModelResolver
// -----------------------------------

UGameItemContainerViewModelResolver::UGameItemContainerViewModelResolver()
{
	ContainerId = UGameItemSettings::GetDefaultContainerId();
}

UObject* UGameItemContainerViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	// return a UGameItemContainerViewModel
	return GetItemContainerViewModel(UserWidget, View);
}

UGameItemContainer* UGameItemContainerViewModelResolver::GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UGameItemsUISubsystem* ItemsUISubsystem = UserWidget->GetWorld()->GetSubsystem<UGameItemsUISubsystem>();
	const FGameItemViewContext Context(UserWidget);
	return ItemsUISubsystem->GetContainerFromProvider(Provider, ContainerId, Context);
}


// UGameItemSlotViewModelResolver
// ------------------------------

UGameItemSlotViewModelResolver::UGameItemSlotViewModelResolver()
{
	Slot = 0;
}

UObject* UGameItemSlotViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	// get container view model
	const UGameItemContainerViewModel* ContainerViewModel = GetItemContainerViewModel(UserWidget, View);
	if (!ContainerViewModel)
	{
		return nullptr;
	}

	// get slot view model from container view model
	TArray<UGameItemSlotViewModel*> SlotViewModels = ContainerViewModel->GetSlotViewModels();
	if (!SlotViewModels.IsValidIndex(Slot))
	{
		return nullptr;
	}
	return SlotViewModels[Slot];
}

UGameItemContainer* UGameItemSlotViewModelResolver::GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UGameItemsUISubsystem* ItemsUISubsystem = UserWidget->GetWorld()->GetSubsystem<UGameItemsUISubsystem>();
	const FGameItemViewContext Context(UserWidget);
	return ItemsUISubsystem->GetContainerFromProvider(Provider, ContainerId, Context);
}
