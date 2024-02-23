// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemCheatsExtension.h"

#include "GameItemDef.h"
#include "GameItemsModule.h"
#include "GameItemSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"


void UGameItemCheatsExtension::GiveItem(TSubclassOf<UGameItemDef> ItemDef, int32 Count, AActor* Actor)
{
	if (!ItemDef)
	{
		UE_LOG(LogGameItems, Warning, TEXT("Could not find item to give"));
		return;
	}

	if (!Actor)
	{
		Actor = GetOuterUCheatManager()->GetPlayerController()->GetPawn();
	}
	if (!Actor)
	{
		return;
	}

	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);

	UGameItemContainer* Container = ItemSubsystem->GetDefaultContainerForActor(Actor);
	if (!Container)
	{
		return;
	}

	ItemSubsystem->CreateItemInContainer(Container, ItemDef, Count);
}
