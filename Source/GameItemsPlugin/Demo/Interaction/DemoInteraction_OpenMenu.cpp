// Copyright Bohdon Sayre, All Rights Reserved.


#include "DemoInteraction_OpenMenu.h"

#include "CommonActivatableWidget.h"
#include "DemoInteractionContextInterface.h"
#include "Demo/Player/ItemsDemoPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Widgets/CommonActivatableWidgetContainer.h"


UDemoInteraction_OpenMenu::UDemoInteraction_OpenMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UDemoInteraction_OpenMenu::Trigger_Implementation(const FDemoInteractionContext& Context)
{
	if (const AItemsDemoPlayerController* DemoPlayer = Context.Interactor->GetController<AItemsDemoPlayerController>())
	{
		if (UCommonActivatableWidgetStack* UILayer = DemoPlayer->GetUILayer(MenuLayerTag))
		{
			UCommonActivatableWidget* MenuWidget = UILayer->AddWidget(MenuClass);
			// pass through context for the menu to use
			SetInteractionContext(MenuWidget, Context);
			return MenuWidget != nullptr;
		}
	}
	return false;
}
