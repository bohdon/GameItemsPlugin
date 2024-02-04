// Copyright Bohdon Sayre, All Rights Reserved.


#include "UIMainLayout.h"


void UUIMainLayout::RegisterUILayer(FGameplayTag LayerTag, UCommonActivatableWidgetStack* Stack)
{
	UILayers.Add(LayerTag, Stack);
}

UCommonActivatableWidgetStack* UUIMainLayout::GetUILayer(FGameplayTag LayerTag) const
{
	return UILayers.FindRef(LayerTag);
}
