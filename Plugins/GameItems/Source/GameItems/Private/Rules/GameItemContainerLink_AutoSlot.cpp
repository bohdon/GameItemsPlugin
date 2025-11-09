// Copyright Bohdon Sayre, All Rights Reserved.


#include "Rules/GameItemContainerLink_AutoSlot.h"

#include "GameItemContainer.h"
#include "GameItemsModule.h"
#include "Net/UnrealNetwork.h"
#include "Rules/GameItemAutoSlotRule_Basic.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerLink_AutoSlot)


UGameItemContainerLink_AutoSlot::UGameItemContainerLink_AutoSlot()
	: ContextTags(TAG_Item_AutoSlot_NoReplace)
{
}

void UGameItemContainerLink_AutoSlot::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerLink_AutoSlot, ItemQuery, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UGameItemContainerLink_AutoSlot, ContextTags, SharedParams);
}

void UGameItemContainerLink_AutoSlot::OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer)
{
	Super::OnLinkedContainerChanged(NewContainer, OldContainer);

	if (OldContainer)
	{
		OldContainer->OnItemAddedEvent.RemoveAll(this);
	}

	if (NewContainer)
	{
		NewContainer->OnItemAddedEvent.AddUObject(this, &UGameItemContainerLink_AutoSlot::OnLinkedItemAdded);
	}
}

void UGameItemContainerLink_AutoSlot::OnLinkedItemAdded(UGameItem* Item)
{
	UGameItemContainer* Container = GetContainer();
	check(Container);
	if (Container->CanAutoSlot(Item, ContextTags) && (ItemQuery.IsEmpty() || ItemQuery.Matches(Item->GetOwnedTags())))
	{
		UE_LOG(LogGameItems, VeryVerbose, TEXT("[%s] Auto-slotting item from link: %s (%s)"),
		       *Container->GetReadableName(), *Item->ToDebugString(), *LinkedContainerId.ToString());
		Container->TryAutoSlot(Item, ContextTags);
	}
}
