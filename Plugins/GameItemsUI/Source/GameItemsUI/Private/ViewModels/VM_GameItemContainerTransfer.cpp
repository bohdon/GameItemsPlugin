// Copyright Bohdon Sayre, All Rights Reserved.

#include "ViewModels/VM_GameItemContainerTransfer.h"

#include "GameItemControllerComponent.h"
#include "GameItemsModule.h"
#include "GameItemSubsystem.h"
#include "GameItemsUISubsystem.h"
#include "Engine/World.h"
#include "ViewModels/VM_GameItemSlot.h"


void UVM_GameItemContainerTransfer::SetOwningPlayer(APlayerController* Player)
{
	OwningPlayer = Player;
}

void UVM_GameItemContainerTransfer::SetContainerA(UGameItemContainer* NewContainer)
{
	UE_MVVM_SET_PROPERTY_VALUE(ContainerA, NewContainer);
}

void UVM_GameItemContainerTransfer::SetContainerB(UGameItemContainer* NewContainer)
{
	UE_MVVM_SET_PROPERTY_VALUE(ContainerB, NewContainer);
}

UGameItemsUISubsystem* UVM_GameItemContainerTransfer::GetItemsUISubsystem() const
{
	return UGameItemsUISubsystem::GetFromPlayerController(OwningPlayer);
}

UGameItemControllerComponent* UVM_GameItemContainerTransfer::GetGameItemController() const
{
	const UGameItemsUISubsystem* ItemsUISubsystem = GetItemsUISubsystem();
	return ItemsUISubsystem ? ItemsUISubsystem->GetGameItemController() : nullptr;
}

void UVM_GameItemContainerTransfer::MoveItem(UVM_GameItemSlot* SlotViewModel, bool bAllowPartial)
{
	if (AreContainersValid() && SlotViewModel &&
		(SlotViewModel->GetContainer() == ContainerA || SlotViewModel->GetContainer() == ContainerB))
	{
		UGameItemContainer* OtherContainer = SlotViewModel->GetContainer() == ContainerA ? ContainerB : ContainerA;
		if (const UGameItemsUISubsystem* ItemsUISubsystem = GetItemsUISubsystem())
		{
			ItemsUISubsystem->MoveItem(SlotViewModel, OtherContainer, bAllowPartial);
		}
		else
		{
			UE_LOG(LogGameItems, Warning,
			       TEXT("[%hs] Couldn't get UGameItemsUISubsystem from OwningPlayer, make sure it's set. Falling back to UGameItemSubsystem calls,"
				       " but this is deprecated and will be removed. Set the OwningPlayer or use UGameItemsUISubsystem::CreateTransferViewModel."),
			       __FUNCTION__);

			UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::Get(SlotViewModel->GetContainer()->GetWorld());
			ItemSubsystem->MoveItem(SlotViewModel->GetContainer(), OtherContainer, SlotViewModel->GetItem(), INDEX_NONE, bAllowPartial);
		}
	}
	else
	{
		UE_CLOG(!ContainerA, LogGameItems, Warning, TEXT("[%hs] ContainerA is not set"), __FUNCTION__);
		UE_CLOG(!ContainerB, LogGameItems, Warning, TEXT("[%hs] ContainerB is not set"), __FUNCTION__);
	}
}

void UVM_GameItemContainerTransfer::MoveAllItemsToA(bool bAllowPartial)
{
	if (AreContainersValid())
	{
		if (UGameItemControllerComponent* Controller = GetGameItemController())
		{
			Controller->MoveAllItems(ContainerB, ContainerA, bAllowPartial);
		}
		else
		{
			UE_LOG(LogGameItems, Warning,
			       TEXT("[%hs] Couldn't get UGameItemsUISubsystem from OwningPlayer, make sure it's set. Falling back to UGameItemSubsystem calls,"
				       " but this is deprecated and will be removed. Set the OwningPlayer or use UGameItemsUISubsystem::CreateTransferViewModel."),
			       __FUNCTION__);

			UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::Get(ContainerA->GetWorld());
			ItemSubsystem->MoveAllItems(ContainerB, ContainerA, bAllowPartial);
		}
	}
	else
	{
		UE_CLOG(!ContainerA, LogGameItems, Warning, TEXT("[%hs] ContainerA is not set"), __FUNCTION__);
		UE_CLOG(!ContainerB, LogGameItems, Warning, TEXT("[%hs] ContainerB is not set"), __FUNCTION__);
	}
}

void UVM_GameItemContainerTransfer::MoveAllItemsToB(bool bAllowPartial)
{
	if (AreContainersValid())
	{
		if (UGameItemControllerComponent* Controller = GetGameItemController())
		{
			Controller->MoveAllItems(ContainerA, ContainerB, bAllowPartial);
		}
		else
		{
			UE_LOG(LogGameItems, Warning,
			       TEXT("[%hs] Couldn't get UGameItemsUISubsystem from OwningPlayer, make sure it's set. Falling back to UGameItemSubsystem calls,"
				       " but this is deprecated and will be removed. Set the OwningPlayer or use UGameItemsUISubsystem::CreateTransferViewModel."),
			       __FUNCTION__);

			UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::Get(ContainerA->GetWorld());
			ItemSubsystem->MoveAllItems(ContainerA, ContainerB, bAllowPartial);
		}
	}
	else
	{
		UE_CLOG(!ContainerA, LogGameItems, Warning, TEXT("[%hs] ContainerA is not set"), __FUNCTION__);
		UE_CLOG(!ContainerB, LogGameItems, Warning, TEXT("[%hs] ContainerB is not set"), __FUNCTION__);
	}
}
