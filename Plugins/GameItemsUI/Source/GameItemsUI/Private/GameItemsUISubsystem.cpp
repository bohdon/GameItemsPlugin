// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemsUISubsystem.h"

#include "GameItemContainer.h"
#include "GameItemSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ViewModels/GameItemSlotViewModel.h"
#include "ViewModels/GameItemContainerViewModel.h"
#include "ViewModels/GameItemViewModel.h"


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

UGameItem* UGameItemsUISubsystem::GetItemFromObject(UObject* ItemObject) const
{
	if (UGameItem* GameItem = Cast<UGameItem>(ItemObject))
	{
		return GameItem;
	}
	else if (const UGameItemViewModel* ItemViewModel = Cast<UGameItemViewModel>(ItemObject))
	{
		return ItemViewModel->GetItem();
	}
	else if (const UGameItemSlotViewModel* SlotViewModel = Cast<UGameItemSlotViewModel>(ItemObject))
	{
		return SlotViewModel->GetItem();
	}
	return nullptr;
}

void UGameItemsUISubsystem::GetContainerAndItem(UObject* ViewModelObject, bool& bSuccess, UGameItemContainer*& Container, UGameItem*& Item) const
{
	if (const UGameItemSlotViewModel* SlotViewModel = Cast<UGameItemSlotViewModel>(ViewModelObject))
	{
		Container = SlotViewModel->GetContainer();
		Item = SlotViewModel->GetItem();
		bSuccess = Item && Container;
	}
	else
	{
		Container = nullptr;
		Item = nullptr;
		bSuccess = false;
	}
}

UGameItemContainer* UGameItemsUISubsystem::GetContainerFromProvider(TSubclassOf<UGameItemContainerProvider> Provider,
                                                                    const FGameplayTag& ContainerId,
                                                                    const FGameItemViewContext& Context)
{
	if (Provider)
	{
		if (const UGameItemContainerProvider* ProviderCDO = GetDefault<UGameItemContainerProvider>(Provider))
		{
			return ProviderCDO->ProvideContainer(ContainerId, Context);
		}
	}
	return nullptr;
}

UGameItemContainerViewModel* UGameItemsUISubsystem::CreateContainerViewModel(UGameItemContainer* Container)
{
	check(Container);
	UGameItemContainerViewModel* NewViewModel = NewObject<UGameItemContainerViewModel>(this);
	NewViewModel->SetContainer(Container);
	return NewViewModel;
}
