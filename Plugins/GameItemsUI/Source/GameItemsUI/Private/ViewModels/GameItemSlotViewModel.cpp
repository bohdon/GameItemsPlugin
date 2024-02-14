// Copyright Bohdon Sayre, All Rights Reserved.


#include "ViewModels/GameItemSlotViewModel.h"

#include "GameItemContainer.h"
#include "GameItemContainerDef.h"
#include "GameItemSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"


UGameItemSlotViewModel::UGameItemSlotViewModel()
	: Container(nullptr),
	  Slot(INDEX_NONE),
	  Item(nullptr)
{
}

UWorld* UGameItemSlotViewModel::GetWorld() const
{
	const UObject* Outer = GetOuter();
	return Outer ? Outer->GetWorld() : nullptr;
}

void UGameItemSlotViewModel::SetContainerAndSlot(UGameItemContainer* NewContainer, int32 NewSlot)
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
			Container->OnItemSlotChangedEvent.AddUObject(this, &UGameItemSlotViewModel::OnItemSlotChanged);
			Container->OnItemSlotsChangedEvent.AddUObject(this, &UGameItemSlotViewModel::OnItemSlotsChanged);
			Container->OnNumSlotsChangedEvent.AddUObject(this, &UGameItemSlotViewModel::OnNumSlotsChanged);
		}

		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Container);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Slot);

		UpdateItem();
	}
}

bool UGameItemSlotViewModel::HasItem() const
{
	return Item != nullptr;
}

bool UGameItemSlotViewModel::IsValidSlot() const
{
	return Container && Container->IsValidSlot(Slot);
}

TArray<UGameItem*> UGameItemSlotViewModel::MoveItem(UGameItemContainer* ToContainer, bool bAllowPartial)
{
	if (!Container || !Item)
	{
		return TArray<UGameItem*>();
	}

	UGameItemSubsystem* ItemSubsystem = UGameInstance::GetSubsystem<UGameItemSubsystem>(GetWorld()->GetGameInstance());
	return ItemSubsystem->MoveItem(Container, ToContainer, Item, -1, bAllowPartial);
}

void UGameItemSlotViewModel::UpdateItem()
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

void UGameItemSlotViewModel::OnItemSlotChanged(int32 InSlot)
{
	if (Slot == InSlot)
	{
		UpdateItem();
	}
}

void UGameItemSlotViewModel::OnItemSlotsChanged(int32 StartSlot, int32 EndSlot)
{
	if (StartSlot <= Slot && Slot <= EndSlot)
	{
		UpdateItem();
	}
}

void UGameItemSlotViewModel::OnNumSlotsChanged(int32 NewNumSlots, int32 OldNumSlots)
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

TArray<UGameItemSlotViewModel*> UGameItemSlotViewModel::CreateSlotViewModelsForContainer(UObject* Outer, UGameItemContainer* InContainer)
{
	TArray<UGameItemSlotViewModel*> Result;
	if (!Outer || !InContainer)
	{
		return TArray<UGameItemSlotViewModel*>();
	}

	const int32 NumSlots = InContainer->GetNumSlots();
	for (int32 Idx = 0; Idx < NumSlots; ++Idx)
	{
		UGameItemSlotViewModel* NewViewModel = NewObject<UGameItemSlotViewModel>(Outer, NAME_None, RF_Transient);
		NewViewModel->SetContainerAndSlot(InContainer, Idx);
		Result.Add(NewViewModel);
	}

	return Result;
}
