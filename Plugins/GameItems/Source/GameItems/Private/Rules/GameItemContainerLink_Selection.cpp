// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerLink_Selection.h"

#include "GameItemContainer.h"


UGameItemContainerLink_Selection::UGameItemContainerLink_Selection()
	: TargetSlot(0),
	  bAllowSelectingEmptySlots(false),
	  SelectedSlot(0)
{
}

void UGameItemContainerLink_Selection::OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer)
{
	Super::OnLinkedContainerChanged(NewContainer, OldContainer);

	if (OldContainer)
	{
		OldContainer->OnItemSlotChangedEvent.RemoveAll(this);
		OldContainer->OnItemAddedEvent.RemoveAll(this);
	}

	if (NewContainer)
	{
		NewContainer->OnItemSlotChangedEvent.AddUObject(this, &UGameItemContainerLink_Selection::OnLinkedSlotChanged);
		NewContainer->OnItemAddedEvent.AddUObject(this, &UGameItemContainerLink_Selection::OnLinkedItemAdded);
	}
}

void UGameItemContainerLink_Selection::SetSelectedSlot(int32 NewSlot)
{
	if (SelectedSlot != NewSlot)
	{
		SelectedSlot = NewSlot;

		UpdateContainerForSelection();
	}
}

void UGameItemContainerLink_Selection::SelectNextSlot(bool bLoop)
{
	if (bAllowSelectingEmptySlots)
	{
		SetSelectedSlot(ClampSlot(SelectedSlot + 1, bLoop));
	}
	else
	{
		SelectNextItem(bLoop);
	}
}

void UGameItemContainerLink_Selection::SelectPrevSlot(bool bLoop)
{
	if (bAllowSelectingEmptySlots)
	{
		SetSelectedSlot(ClampSlot(SelectedSlot - 1, bLoop));
	}
	else
	{
		SelectPrevItem(bLoop);
	}
}

void UGameItemContainerLink_Selection::SelectNextItem(bool bLoop)
{
	const int32 NewSlot = FindValidItemSlot(1, bLoop);
	if (NewSlot != INDEX_NONE)
	{
		SetSelectedSlot(NewSlot);
	}
}

void UGameItemContainerLink_Selection::SelectPrevItem(bool bLoop)
{
	const int32 NewSlot = FindValidItemSlot(-1, bLoop);
	if (NewSlot != INDEX_NONE)
	{
		SetSelectedSlot(NewSlot);
	}
}

int32 UGameItemContainerLink_Selection::FindValidItemSlot(int32 SearchDirection, bool bLoop) const
{
	if (!LinkedContainer)
	{
		return INDEX_NONE;
	}

	if (bLoop)
	{
		// iterate the full number of slots in the container
		const int32 NumSlots = LinkedContainer->GetNumSlots();
		for (int32 Idx = 1; Idx < NumSlots; ++Idx)
		{
			const int32 Slot = (SelectedSlot + (Idx * SearchDirection) + NumSlots) % NumSlots;
			if (!LinkedContainer->IsSlotEmpty(Slot))
			{
				return Slot;
			}
		}
	}
	else
	{
		// iterate just to the end
		if (SearchDirection > 0)
		{
			const int32 NumSlots = LinkedContainer->GetNumSlots();
			for (int32 Slot = SelectedSlot + 1; Slot < NumSlots; ++Slot)
			{
				if (!LinkedContainer->IsSlotEmpty(Slot))
				{
					return Slot;
				}
			}
		}
		else
		{
			for (int32 Slot = SelectedSlot - 1; Slot >= 0; --Slot)
			{
				if (!LinkedContainer->IsSlotEmpty(Slot))
				{
					return Slot;
				}
			}
		}
	}

	return INDEX_NONE;
}

int32 UGameItemContainerLink_Selection::ClampSlot(int32 Slot, bool bLoop) const
{
	if (!LinkedContainer || LinkedContainer->GetNumSlots() == 0)
	{
		return 0;
	}

	if (bLoop)
	{
		const int32 NumSlots = LinkedContainer->GetNumSlots();
		return (Slot + NumSlots) % NumSlots;
	}

	return FMath::Clamp(Slot, 0, LinkedContainer->GetNumSlots());
}

void UGameItemContainerLink_Selection::OnLinkedSlotChanged(int32 Slot)
{
	if (Slot == SelectedSlot && LinkedContainer)
	{
		if (LinkedContainer->IsSlotEmpty(SelectedSlot) && !bAllowSelectingEmptySlots)
		{
			// clear the item now in case no valid item is found, and the selected slot doesn't change
			Container->RemoveItemAt(TargetSlot);
			SelectNextItem();
			return;
		}

		UpdateContainerForSelection();
	}
}

void UGameItemContainerLink_Selection::OnLinkedItemAdded(UGameItem* GameItem)
{
	if (!bAllowSelectingEmptySlots && Container->IsSlotEmpty(TargetSlot))
	{
		// try to select the new item
		SelectNextItem(true);
	}
}

void UGameItemContainerLink_Selection::UpdateContainerForSelection()
{
	if (!LinkedContainer)
	{
		return;
	}

	UGameItem* SelectedItem = LinkedContainer->GetItemAt(SelectedSlot);
	if (Container->GetItemAt(TargetSlot) != SelectedItem)
	{
		Container->RemoveItemAt(TargetSlot);

		if (SelectedItem)
		{
			Container->AddItem(SelectedItem, TargetSlot);
		}
	}
}
