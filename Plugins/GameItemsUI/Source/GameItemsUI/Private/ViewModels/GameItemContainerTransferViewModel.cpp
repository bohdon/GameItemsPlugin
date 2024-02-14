// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/GameItemContainerTransferViewModel.h"

#include "GameItemSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ViewModels/GameItemSlotViewModel.h"


void UGameItemContainerTransferViewModel::SetContainerA(UGameItemContainer* NewContainer)
{
	UE_MVVM_SET_PROPERTY_VALUE(ContainerA, NewContainer);
}

void UGameItemContainerTransferViewModel::SetContainerB(UGameItemContainer* NewContainer)
{
	UE_MVVM_SET_PROPERTY_VALUE(ContainerB, NewContainer);
}

void UGameItemContainerTransferViewModel::MoveItem(UGameItemSlotViewModel* SlotViewModel, bool bAllowPartial)
{
	if (AreContainersValid() && SlotViewModel &&
		SlotViewModel->GetContainer() == ContainerA || SlotViewModel->GetContainer() == ContainerB)
	{
		UGameItemContainer* OtherContainer = SlotViewModel->GetContainer() == ContainerA ? ContainerB : ContainerA;
		SlotViewModel->MoveItem(OtherContainer, bAllowPartial);
	}
}

void UGameItemContainerTransferViewModel::MoveAllItemsToA(bool bAllowPartial)
{
	if (AreContainersValid())
	{
		UGameItemSubsystem* ItemSubsystem = UGameInstance::GetSubsystem<UGameItemSubsystem>(GetWorld()->GetGameInstance());
		ItemSubsystem->MoveAllItems(ContainerB, ContainerA, bAllowPartial);
	}
}

void UGameItemContainerTransferViewModel::MoveAllItemsToB(bool bAllowPartial)
{
	if (AreContainersValid())
	{
		UGameItemSubsystem* ItemSubsystem = UGameInstance::GetSubsystem<UGameItemSubsystem>(GetWorld()->GetGameInstance());
		ItemSubsystem->MoveAllItems(ContainerA, ContainerB, bAllowPartial);
	}
}
