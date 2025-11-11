// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/VM_GameItemContainerTransfer.h"

#include "GameItemSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ViewModels/VM_GameItemSlot.h"


void UVM_GameItemContainerTransfer::SetContainerA(UGameItemContainer* NewContainer)
{
	UE_MVVM_SET_PROPERTY_VALUE(ContainerA, NewContainer);
}

void UVM_GameItemContainerTransfer::SetContainerB(UGameItemContainer* NewContainer)
{
	UE_MVVM_SET_PROPERTY_VALUE(ContainerB, NewContainer);
}

void UVM_GameItemContainerTransfer::MoveItem(UVM_GameItemSlot* SlotViewModel, bool bAllowPartial)
{
	if (AreContainersValid() && SlotViewModel &&
		(SlotViewModel->GetContainer() == ContainerA || SlotViewModel->GetContainer() == ContainerB))
	{
		UGameItemContainer* OtherContainer = SlotViewModel->GetContainer() == ContainerA ? ContainerB : ContainerA;
		SlotViewModel->MoveItem(OtherContainer, bAllowPartial);
	}
}

void UVM_GameItemContainerTransfer::MoveAllItemsToA(bool bAllowPartial)
{
	if (AreContainersValid())
	{
		UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
		ItemSubsystem->MoveAllItems(ContainerB, ContainerA, bAllowPartial);
	}
}

void UVM_GameItemContainerTransfer::MoveAllItemsToB(bool bAllowPartial)
{
	if (AreContainersValid())
	{
		UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
		ItemSubsystem->MoveAllItems(ContainerA, ContainerB, bAllowPartial);
	}
}
