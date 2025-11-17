// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerProvider.h"

#include "GameItemContainerInterface.h"
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

	const APlayerController* Player = Context.UserWidget->GetOwningPlayer();
	if (!Player)
	{
		return nullptr;
	}

	const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(Player);

	// check player state first, it's common to put item container components on this directly 
	const IGameItemContainerInterface* ContainerInterface = ContainerInterface = ItemSubsystem->GetContainerInterfaceForActor(Player->PlayerState);

	if (!ContainerInterface)
	{
		// check player controller 2nd, it's more likely to be initialized before pawn
		ContainerInterface = ItemSubsystem->GetContainerInterfaceForActor(Player);
	}
	if (!ContainerInterface)
	{
		// check pawn last
		ContainerInterface = ItemSubsystem->GetContainerInterfaceForActor(Player->GetPawn());
	}
	if (!ContainerInterface)
	{
		// nothing found
		return nullptr;
	}

	return ContainerInterface->GetItemContainer(ContainerId);
}
