// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemsUISubsystem.h"

#include "GameItemContainer.h"
#include "GameItemControllerComponent.h"
#include "GameItemsModule.h"
#include "GameItemSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "ViewModels/VM_GameItem.h"
#include "ViewModels/VM_GameItemContainer.h"
#include "ViewModels/VM_GameItemContainerTransfer.h"
#include "ViewModels/VM_GameItemSlot.h"


UGameItemsUISubsystem* UGameItemsUISubsystem::GetFromPlayerController(const APlayerController* Player)
{
	const ULocalPlayer* LocalPlayer = Player ? Player->GetLocalPlayer() : nullptr;
	return LocalPlayer ? LocalPlayer->GetSubsystem<UGameItemsUISubsystem>() : nullptr;
}

UGameItemsUISubsystem* UGameItemsUISubsystem::GetFromUserWidget(const UUserWidget* UserWidget)
{
	const ULocalPlayer* LocalPlayer = UserWidget ? UserWidget->GetOwningLocalPlayer() : nullptr;
	return LocalPlayer ? LocalPlayer->GetSubsystem<UGameItemsUISubsystem>() : nullptr;
}

bool UGameItemsUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// only create an instance if there is no overridden implementation
	return ChildClasses.Num() == 0;
}

void UGameItemsUISubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	PlayerController = NewPlayerController;
}

UGameItemControllerComponent* UGameItemsUISubsystem::GetGameItemController() const
{
	return PlayerController ? PlayerController->FindComponentByClass<UGameItemControllerComponent>() : nullptr;
}

void UGameItemsUISubsystem::MoveItem(UVM_GameItemSlot* FromSlot, UGameItemContainer* To, bool bAllowPartial) const
{
	if (!FromSlot || !To)
	{
		return;
	}

	UGameItemContainer* From = FromSlot->GetContainer();
	UGameItem* Item = FromSlot->GetItem();
	if (!From || !Item)
	{
		return;
	}

	if (UGameItemControllerComponent* Controller = GetGameItemController())
	{
		Controller->MoveItem(From, To, Item, bAllowPartial);
	}
	else
	{
		// no network support, but don't require a controller for standalone
		UE_CLOG(GetWorld()->GetNetMode() != NM_Standalone, LogGameItems, Warning,
			TEXT("No UGameItemControllerComponent found, network control may not work as expected"));

		UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
		ItemSubsystem->MoveItem(From, To, Item, INDEX_NONE, bAllowPartial);
	}
}

void UGameItemsUISubsystem::MoveSwapOrStackItem(UVM_GameItemSlot* FromSlot, UVM_GameItemSlot* ToSlot, bool bAllowPartial) const
{
	if (!FromSlot || !ToSlot || FromSlot->IsSameSlot(ToSlot))
	{
		return;
	}

	UGameItemContainer* From = FromSlot->GetContainer();
	UGameItem* Item = FromSlot->GetItem();
	UGameItemContainer* To = ToSlot->GetContainer();
	const int32 ToSlotIdx = ToSlot->GetSlot();
	if (!From || !Item || !To || ToSlotIdx == INDEX_NONE)
	{
		return;
	}

	if (UGameItemControllerComponent* Controller = GetGameItemController())
	{
		Controller->MoveSwapOrStackItem(From, Item, To, ToSlotIdx, bAllowPartial);
	}
	else
	{
		// no network support, but don't require a controller for standalone
		UE_CLOG(GetWorld()->GetNetMode() != NM_Standalone, LogGameItems, Warning,
			TEXT("No UGameItemControllerComponent found, network control may not work as expected"));

		UGameItemSubsystem* ItemsSubsystem = UGameItemSubsystem::Get(this);
		ItemsSubsystem->MoveSwapOrStackItem(From, Item, To, ToSlotIdx, bAllowPartial);
	}
}

UVM_GameItemContainer* UGameItemsUISubsystem::GetOrCreateContainerViewModel(UGameItemContainer* Container)
{
	if (!Container)
	{
		return nullptr;
	}

	const TObjectPtr<UVM_GameItemContainer>* ContainerViewModel = ContainerViewModels.FindByPredicate([Container](const UVM_GameItemContainer* ViewModel)
		{
			return ViewModel && ViewModel->GetContainer() == Container;
		});

	if (ContainerViewModel)
	{
		return ContainerViewModel->Get();
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

UVM_GameItemContainerTransfer* UGameItemsUISubsystem::CreateTransferViewModel(UObject* Outer)
{
	UVM_GameItemContainerTransfer* NewViewModel = NewObject<UVM_GameItemContainerTransfer>(Outer, NAME_None, RF_Transient);
	NewViewModel->SetOwningPlayer(PlayerController);
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

UGameItemContainer* UGameItemsUISubsystem::GetContainerFromProvider(
	TSubclassOf<UGameItemContainerProvider> Provider,
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

UVM_GameItemContainer* UGameItemsUISubsystem::CreateContainerViewModel(UGameItemContainer* Container)
{
	check(Container);
	UVM_GameItemContainer* NewViewModel = NewObject<UVM_GameItemContainer>(this);
	NewViewModel->SetContainer(Container);
	return NewViewModel;
}
