// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemsUISubsystem.h"

#include "GameItemContainer.h"
#include "GameItemSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ViewModels/VM_GameItemSlot.h"
#include "ViewModels/VM_GameItemContainer.h"
#include "ViewModels/VM_GameItem.h"


UVM_GameItemContainer* UGameItemsUISubsystem::GetOrCreateContainerViewModel(UGameItemContainer* Container)
{
	if (!Container)
	{
		return nullptr;
	}

	UVM_GameItemContainer** ContainerViewModel = ContainerViewModels.FindByPredicate([Container](UVM_GameItemContainer* ViewModel)
	{
		return ViewModel && ViewModel->GetContainer() == Container;
	});

	if (ContainerViewModel)
	{
		return *ContainerViewModel;
	}

	// create a new view model
	UVM_GameItemContainer* NewViewModel = CreateContainerViewModel(Container);
	check(NewViewModel);
	ContainerViewModels.Add(NewViewModel);
	return NewViewModel;
}

UVM_GameItemContainer* UGameItemsUISubsystem::GetOrCreateContainerViewModelForActor(AActor* Actor, FGameplayTag ContainerId)
{
	const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
	UGameItemContainer* Container = ItemSubsystem->GetContainerForActor(Actor, ContainerId);
	return GetOrCreateContainerViewModel(Container);
}

UVM_GameItemSlot* UGameItemsUISubsystem::CreateSlotViewModelForContainer(UObject* Outer, UGameItemContainer* InContainer, int32 Slot)
{
	UVM_GameItemSlot* NewViewModel = NewObject<UVM_GameItemSlot>(Outer, NAME_None, RF_Transient);
	NewViewModel->SetContainerAndSlot(InContainer, Slot);
	return NewViewModel;
}

UGameItem* UGameItemsUISubsystem::GetItemFromObject(UObject* ItemObject) const
{
	if (UGameItem* GameItem = Cast<UGameItem>(ItemObject))
	{
		return GameItem;
	}
	else if (const UVM_GameItem* ItemViewModel = Cast<UVM_GameItem>(ItemObject))
	{
		return ItemViewModel->GetItem();
	}
	else if (const UVM_GameItemSlot* SlotViewModel = Cast<UVM_GameItemSlot>(ItemObject))
	{
		return SlotViewModel->GetItem();
	}
	return nullptr;
}

void UGameItemsUISubsystem::GetContainerAndItem(UObject* ViewModelObject, bool& bSuccess, UGameItemContainer*& Container, UGameItem*& Item) const
{
	if (const UVM_GameItemSlot* SlotViewModel = Cast<UVM_GameItemSlot>(ViewModelObject))
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

void UGameItemsUISubsystem::MoveSwapOrStackItem(UVM_GameItemSlot* FromSlot, UVM_GameItemSlot* ToSlot, bool bAllowPartial) const
{
	if (!FromSlot || !FromSlot->GetContainer() || !FromSlot->GetItem() || !ToSlot || !ToSlot->GetContainer())
	{
		return;
	}

	if (FromSlot->GetContainer() == ToSlot->GetContainer())
	{
		if (FromSlot->GetSlot() == ToSlot->GetSlot())
		{
			// same slot
			return;
		}

		if (FromSlot->GetItem()->IsMatching(ToSlot->GetItem()) && !ToSlot->GetContainer()->IsStackFull(ToSlot->GetSlot()))
		{
			// stack items
			ToSlot->GetContainer()->StackItems(FromSlot->GetSlot(), ToSlot->GetSlot(), bAllowPartial);
		}
		else
		{
			// swap items in the container
			ToSlot->GetContainer()->SwapItems(FromSlot->GetSlot(), ToSlot->GetSlot());
		}
	}
	else if (ToSlot->GetContainer()->IsChild())
	{
		// assign / replace item to a child container
		if (ToSlot->GetItem())
		{
			ToSlot->GetContainer()->RemoveItemAt(ToSlot->GetSlot());
		}
		const int32 ExistingItemSlot = ToSlot->GetContainer()->GetItemSlot(FromSlot->GetItem());
		if (ExistingItemSlot != INDEX_NONE)
		{
			// re-assigning an item from parent container, just move the item to the new location
			ToSlot->GetContainer()->SwapItems(ExistingItemSlot, ToSlot->GetSlot());
		}
		else
		{
			// assign new item
			ToSlot->GetContainer()->AddItem(FromSlot->GetItem(), ToSlot->GetSlot());
		}
	}
	else
	{
		// move from another container
		UGameItemSubsystem* ItemsSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
		ItemsSubsystem->MoveItem(FromSlot->GetContainer(), ToSlot->GetContainer(), FromSlot->GetItem(), ToSlot->GetSlot(), bAllowPartial);
	}
}

UVM_GameItemContainer* UGameItemsUISubsystem::CreateContainerViewModel(UGameItemContainer* Container)
{
	check(Container);
	UVM_GameItemContainer* NewViewModel = NewObject<UVM_GameItemContainer>(this);
	NewViewModel->SetContainer(Container);
	return NewViewModel;
}
