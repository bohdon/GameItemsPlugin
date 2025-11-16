// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/VM_GameItemSlot.h"

#include "GameItemContainer.h"
#include "GameItemContainerDef.h"
#include "GameItemSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"


UVM_GameItemSlot::UVM_GameItemSlot()
	: Container(nullptr),
	  Slot(INDEX_NONE),
	  Item(nullptr)
{
}

UWorld* UVM_GameItemSlot::GetWorld() const
{
	const UObject* Outer = GetOuter();
	return Outer ? Outer->GetWorld() : nullptr;
}

void UVM_GameItemSlot::SetContainerAndSlot(UGameItemContainer* NewContainer, int32 NewSlot)
{
	if (Container != NewContainer || Slot != NewSlot)
	{
		if (Container)
		{
			Container->OnItemSlotChangedEvent.RemoveAll(this);
			Container->OnItemSlotsChangedEvent.RemoveAll(this);
			Container->OnNumSlotsChangedEvent.RemoveAll(this);
		}

		Container = NewContainer;
		Slot = NewSlot;

		if (Container)
		{
			Container->OnItemSlotChangedEvent.AddUObject(this, &UVM_GameItemSlot::OnItemSlotChanged);
			Container->OnItemSlotsChangedEvent.AddUObject(this, &UVM_GameItemSlot::OnItemSlotsChanged);
			Container->OnNumSlotsChangedEvent.AddUObject(this, &UVM_GameItemSlot::OnNumSlotsChanged);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Container);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Slot);

		UpdateItem();
	}
}

bool UVM_GameItemSlot::HasItem() const
{
	return Item != nullptr;
}

bool UVM_GameItemSlot::IsValidSlot() const
{
	return Container && Container->IsValidSlot(Slot);
}

void UVM_GameItemSlot::MoveItem(UGameItemContainer* ToContainer, bool bAllowPartial)
{
	if (!Container || !Item)
	{
		return;
	}

	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
	ItemSubsystem->MoveItem(Container, ToContainer, Item, -1, bAllowPartial);
}

void UVM_GameItemSlot::UpdateItem()
{
	UGameItem* NewItem = nullptr;
	if (Container && Slot != INDEX_NONE)
	{
		NewItem = Container->GetItemAt(Slot);
	}

	if (Item != NewItem)
	{
		Item = NewItem;

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Item);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasItem);
	}
}

void UVM_GameItemSlot::OnItemSlotChanged(int32 InSlot)
{
	if (Slot == InSlot)
	{
		UpdateItem();
	}
}

void UVM_GameItemSlot::OnItemSlotsChanged(int32 StartSlot, int32 EndSlot)
{
	if (StartSlot <= Slot && Slot <= EndSlot)
	{
		UpdateItem();
	}
}

void UVM_GameItemSlot::OnNumSlotsChanged(int32 NewNumSlots, int32 OldNumSlots)
{
	if (Slot >= NewNumSlots)
	{
		// slot has been removed
		Item = nullptr;

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Item);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HasItem);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsValidSlot);
	}
	if (Slot >= OldNumSlots && Slot < NewNumSlots)
	{
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(IsValidSlot);
	}
}

TArray<UVM_GameItemSlot*> UVM_GameItemSlot::CreateSlotViewModelsForContainer(UObject* Outer, UGameItemContainer* InContainer)
{
	TArray<UVM_GameItemSlot*> Result;
	if (!Outer || !InContainer)
	{
		return TArray<UVM_GameItemSlot*>();
	}

	const int32 NumSlots = InContainer->GetNumSlots();
	for (int32 Idx = 0; Idx < NumSlots; ++Idx)
	{
		UVM_GameItemSlot* NewViewModel = NewObject<UVM_GameItemSlot>(Outer, NAME_None, RF_Transient);
		NewViewModel->SetContainerAndSlot(InContainer, Idx);
		Result.Add(NewViewModel);
	}

	return Result;
}
