// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoInteraction_OpenMenu.h"

#include "CommonActivatableWidget.h"
#include "Demo/Player/ItemsDemoPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Widgets/CommonActivatableWidgetContainer.h"


UDemoInteraction_OpenMenu::UDemoInteraction_OpenMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UDemoInteraction_OpenMenu::Trigger_Implementation(APawn* Interactor)
{
	if (const AItemsDemoPlayerController* DemoPlayer = Interactor->GetController<AItemsDemoPlayerController>())
	{
		if (UCommonActivatableWidgetStack* UILayer = DemoPlayer->GetUILayer(MenuLayerTag))
		{
			const UCommonActivatableWidget* ScreenWidget = UILayer->AddWidget(MenuClass);
			return ScreenWidget != nullptr;
		}
	}
	return false;
}
