// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerProvider.h"

#include "GameItemContainerInterface.h"
#include "GameItemStatics.h"
#include "GameItemSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"


// UGameItemContainerProvider
// --------------------------

UGameItemContainer* UGameItemContainerProvider::ProvideContainer_Implementation(const FGameplayTag& ContainerId,
                                                                                const FGameItemViewContext& Context) const
{
	return nullptr;
}


// UGameItemContainerProvider_Player
// ---------------------------------

UGameItemContainer* UGameItemContainerProvider_Player::ProvideContainer_Implementation(const FGameplayTag& ContainerId,
                                                                                       const FGameItemViewContext& Context) const
{
	if (!Context.UserWidget)
	{
		return nullptr;
	}
	APlayerController* Player = Context.UserWidget->GetOwningPlayer();
	if (!Player)
	{
		return nullptr;
	}

	const UGameItemSubsystem* ItemSubsystem = UGameItemStatics::GetItemSubsystemFromContextObject(Player);
	const IGameItemContainerInterface* ContainerInterface = ItemSubsystem->GetContainerInterfaceForActor(Player->GetPawn());
	if (!ContainerInterface)
	{
		ContainerInterface = ItemSubsystem->GetContainerInterfaceForActor(Player->PlayerState);
	}
	if (!ContainerInterface)
	{
		ContainerInterface = ItemSubsystem->GetContainerInterfaceForActor(Player);
	}
	if (!ContainerInterface)
	{
		// nothing found
		return nullptr;
	}

	return ContainerInterface->GetItemContainer(ContainerId);
}
