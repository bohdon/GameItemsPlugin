// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoPlayerController.h"

#include "ItemsDemoHUD.h"


UCommonActivatableWidgetStack* AItemsDemoPlayerController::GetUILayer(FGameplayTag LayerTag) const
{
	if (const AItemsDemoHUD* DemoHUD = GetHUD<AItemsDemoHUD>())
	{
		return DemoHUD->GetUILayer(LayerTag);
	}
	return nullptr;
}
