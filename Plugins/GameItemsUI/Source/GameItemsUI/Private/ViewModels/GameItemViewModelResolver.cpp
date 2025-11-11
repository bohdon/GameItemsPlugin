// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/GameItemViewModelResolver.h"

#include "GameItemContainerProvider.h"
#include "GameItemSettings.h"
#include "GameItemsUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "ViewModels/VM_GameItemContainer.h"
#include "ViewModels/VM_GameItemSlot.h"


// UGameItemViewModelResolverBase
// ------------------------------

UVM_GameItemContainer* UGameItemViewModelResolverBase::GetItemContainerViewModel(const UUserWidget* UserWidget, const UMVVMView* View) const
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

UVMR_GameItemContainer::UVMR_GameItemContainer()
{
	ContainerId = UGameItemSettings::GetDefaultContainerId();
}

UObject* UVMR_GameItemContainer::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	// return a UGameItemContainerViewModel
	return GetItemContainerViewModel(UserWidget, View);
}

UGameItemContainer* UVMR_GameItemContainer::GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UGameItemsUISubsystem* ItemsUISubsystem = UserWidget->GetWorld()->GetSubsystem<UGameItemsUISubsystem>();
	const FGameItemViewContext Context(UserWidget);
	return ItemsUISubsystem->GetContainerFromProvider(Provider, ContainerId, Context);
}


// UGameItemSlotViewModelResolver
// ------------------------------

UVMR_GameItemSlot::UVMR_GameItemSlot()
{
	Slot = 0;
}

UObject* UVMR_GameItemSlot::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	// get container view model
	const UVM_GameItemContainer* ContainerViewModel = GetItemContainerViewModel(UserWidget, View);
	if (!ContainerViewModel)
	{
		return nullptr;
	}

	// get slot view model from container view model
	TArray<UVM_GameItemSlot*> SlotViewModels = ContainerViewModel->GetSlotViewModels();
	if (!SlotViewModels.IsValidIndex(Slot))
	{
		return nullptr;
	}
	return SlotViewModels[Slot];
}

UGameItemContainer* UVMR_GameItemSlot::GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const
{
	UGameItemsUISubsystem* ItemsUISubsystem = UserWidget->GetWorld()->GetSubsystem<UGameItemsUISubsystem>();
	const FGameItemViewContext Context(UserWidget);
	return ItemsUISubsystem->GetContainerFromProvider(Provider, ContainerId, Context);
}
