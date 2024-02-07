// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/GameItemContainerViewModel.h"

#include "GameItemContainer.h"
#include "ViewModels/GameItemContainerSlotViewModel.h"


void UGameItemContainerViewModel::SetContainer(UGameItemContainer* NewContainer)
{
	if (Container != NewContainer)
	{
		if (Container)
		{
			Container->OnItemAddedEvent.RemoveAll(this);
			Container->OnItemRemovedEvent.RemoveAll(this);
			Container->OnNumSlotsChangedEvent.RemoveAll(this);
			SlotViewModels.Reset();
		}

		Container = NewContainer;

		if (Container)
		{
			Container->OnItemAddedEvent.AddUObject(this, &UGameItemContainerViewModel::OnItemAdded);
			Container->OnItemRemovedEvent.AddUObject(this, &UGameItemContainerViewModel::OnItemRemoved);
			Container->OnNumSlotsChangedEvent.AddUObject(this, &UGameItemContainerViewModel::OnNumSlotsChanged);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Container);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNumSlots);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItems);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSlotViewModels);
	}
}

int32 UGameItemContainerViewModel::GetNumSlots() const
{
	return Container ? Container->GetNumSlots() : 0;
}

TArray<UGameItem*> UGameItemContainerViewModel::GetItems() const
{
	return Container ? Container->GetAllItems() : TArray<UGameItem*>();
}

TArray<UGameItemContainerSlotViewModel*> UGameItemContainerViewModel::GetSlotViewModels() const
{
	UGameItemContainerViewModel* MutableThis = const_cast<UGameItemContainerViewModel*>(this);

	const int32 OldNumSlots = SlotViewModels.Num();
	const int32 NumSlots = GetNumSlots();
	if (OldNumSlots != NumSlots)
	{
		MutableThis->SlotViewModels.SetNum(NumSlots);

		// create any new models
		for (int32 Slot = OldNumSlots; Slot < NumSlots; ++Slot)
		{
			UGameItemContainerSlotViewModel* SlotViewModel = NewObject<UGameItemContainerSlotViewModel>(MutableThis);
			SlotViewModel->SetContainerAndSlot(Container, Slot);
			MutableThis->SlotViewModels[Slot] = SlotViewModel;
		}
	}

	return SlotViewModels;
}

void UGameItemContainerViewModel::OnItemAdded(UGameItem* Item)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItems);
}

void UGameItemContainerViewModel::OnItemRemoved(UGameItem* Item)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItems);
}

void UGameItemContainerViewModel::OnNumSlotsChanged(int32 NewNumSlots, int32 OldNumSlots)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNumSlots);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSlotViewModels);
}
