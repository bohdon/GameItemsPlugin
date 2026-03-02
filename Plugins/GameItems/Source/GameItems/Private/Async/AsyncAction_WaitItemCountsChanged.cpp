// Copyright Bohdon Sayre, All Rights Reserved.


#include "Async/AsyncAction_WaitItemCountsChanged.h"

#include "GameItemContainer.h"


UAsyncAction_WaitItemCountsChanged* UAsyncAction_WaitItemCountsChanged::WaitForItemCountsChanged(UGameItemContainer* Container)
{
	UAsyncAction_WaitItemCountsChanged* NewAction = NewObject<UAsyncAction_WaitItemCountsChanged>();
	NewAction->Container = Container;
	return NewAction;
}

void UAsyncAction_WaitItemCountsChanged::Activate()
{
	if (!Container.IsValid())
	{
		SetReadyToDestroy();
		return;
	}

	Container->OnItemAddedEvent.AddUObject(this, &ThisClass::OnItemAdded);
	Container->OnItemRemovedEvent.AddUObject(this, &ThisClass::OnItemRemoved);
}

void UAsyncAction_WaitItemCountsChanged::OnItemAdded(UGameItem* Item)
{
	OnCountChanged.Broadcast(Item, Item->GetCount(), 0);

	Item->OnCountChangedEvent.AddUObject(this, &ThisClass::OnItemCountChanged);
}

void UAsyncAction_WaitItemCountsChanged::OnItemRemoved(UGameItem* Item)
{
	OnCountChanged.Broadcast(Item, 0, Item->GetCount());

	Item->OnCountChangedEvent.RemoveAll(this);
}

void UAsyncAction_WaitItemCountsChanged::OnItemCountChanged(UGameItem* Item, int32 NewCount, int32 OldCount)
{
	OnCountChanged.Broadcast(Item, NewCount, OldCount);
}
