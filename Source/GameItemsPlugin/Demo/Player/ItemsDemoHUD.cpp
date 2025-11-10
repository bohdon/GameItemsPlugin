// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoHUD.h"

#include "GameItemContainerComponent.h"
#include "GameItemsModule.h"
#include "TimerManager.h"
#include "Demo/UI/UIMainLayout.h"
#include "UObject/UObjectIterator.h"


void AItemsDemoHUD::GetDebugActorList(TArray<AActor*>& InOutList)
{
	Super::GetDebugActorList(InOutList);

	UWorld* World = GetWorld();

	// if debugging game items, add all actors with game item containers
	if (ShouldDisplayDebug(ShowDebugNames::GameItems))
	{
		for (TObjectIterator<UGameItemContainerComponent> It; It; ++It)
		{
			AddComponentOwnerToDebugList(*It, InOutList, World);
		}
	}
}

UCommonActivatableWidgetStack* AItemsDemoHUD::GetUILayer(FGameplayTag LayerTag) const
{
	return MainLayout->GetUILayer(LayerTag);
}

void AItemsDemoHUD::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_Standalone)
	{
		CreateMainLayout();
	}
	else
	{
		// wait a bit for player state to replicate
		FTimerHandle DelayHandle;
		GetWorldTimerManager().SetTimer(DelayHandle, this, &ThisClass::CreateMainLayout, 0.1f);
	}
}

void AItemsDemoHUD::CreateMainLayout()
{
	if (!MainLayoutClass)
	{
		return;
	}

	MainLayout = CreateWidget<UUIMainLayout>(PlayerOwner, MainLayoutClass);
	MainLayout->AddToPlayerScreen();
}
