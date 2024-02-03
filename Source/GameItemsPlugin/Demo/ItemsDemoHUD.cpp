// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoHUD.h"

#include "GameItemContainerComponent.h"
#include "GameItemsModule.h"
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
