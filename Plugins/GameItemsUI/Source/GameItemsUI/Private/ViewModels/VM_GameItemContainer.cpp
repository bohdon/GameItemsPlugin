// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/VM_GameItemContainer.h"

#include "GameItemContainer.h"
#include "ViewModels/VM_GameItemSlot.h"


void UVM_GameItemContainer::SetContainer(UGameItemContainer* NewContainer)
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
			Container->OnItemAddedEvent.AddUObject(this, &UVM_GameItemContainer::OnItemAdded);
			Container->OnItemRemovedEvent.AddUObject(this, &UVM_GameItemContainer::OnItemRemoved);
			Container->OnNumSlotsChangedEvent.AddUObject(this, &UVM_GameItemContainer::OnNumSlotsChanged);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Container);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNumSlots);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItems);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSlotViewModels);
	}
}

int32 UVM_GameItemContainer::GetNumSlots() const
{
	return Container ? Container->GetNumSlots() : 0;
}

TArray<UGameItem*> UVM_GameItemContainer::GetItems() const
{
	return Container ? Container->GetAllItems() : TArray<UGameItem*>();
}

TArray<UVM_GameItemSlot*> UVM_GameItemContainer::GetSlotViewModels() const
{
	UVM_GameItemContainer* MutableThis = const_cast<UVM_GameItemContainer*>(this);

	const int32 OldNumSlots = SlotViewModels.Num();
	const int32 NumSlots = GetNumSlots();
	if (OldNumSlots != NumSlots)
	{
		MutableThis->SlotViewModels.SetNum(NumSlots);

		// create any new models
		for (int32 Slot = OldNumSlots; Slot < NumSlots; ++Slot)
		{
			UVM_GameItemSlot* SlotViewModel = NewObject<UVM_GameItemSlot>(MutableThis);
			SlotViewModel->SetContainerAndSlot(Container, Slot);
			MutableThis->SlotViewModels[Slot] = SlotViewModel;
		}
	}

	return SlotViewModels;
}

void UVM_GameItemContainer::OnItemAdded(UGameItem* Item)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItems);
}

void UVM_GameItemContainer::OnItemRemoved(UGameItem* Item)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetItems);
}

void UVM_GameItemContainer::OnNumSlotsChanged(int32 NewNumSlots, int32 OldNumSlots)
{
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetNumSlots);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetSlotViewModels);
}
