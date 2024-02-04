// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoHUD.h"

#include "GameItemContainerComponent.h"
#include "GameItemsModule.h"
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
	CreateMainLayout();

	Super::BeginPlay();
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
