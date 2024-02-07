// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoPlayerController.h"

#include "GameItemStatics.h"
#include "ItemsDemoHUD.h"
#include "ItemsDemoPlayerState.h"


UCommonActivatableWidgetStack* AItemsDemoPlayerController::GetUILayer(FGameplayTag LayerTag) const
{
	if (const AItemsDemoHUD* DemoHUD = GetHUD<AItemsDemoHUD>())
	{
		return DemoHUD->GetUILayer(LayerTag);
	}
	return nullptr;
}

TArray<UGameItemContainer*> AItemsDemoPlayerController::GetAllItemContainers() const
{
	return UGameItemStatics::GetAllItemContainersForActor(PlayerState);
}

UGameItemContainer* AItemsDemoPlayerController::GetItemContainer(FGameplayTag ContainerId) const
{
	return UGameItemStatics::GetItemContainerForActor(PlayerState, ContainerId);
}
