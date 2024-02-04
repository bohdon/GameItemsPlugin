// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoPlayerState.h"

#include "GameItemContainerComponent.h"


AItemsDemoPlayerState::AItemsDemoPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameItemContainerComponent = CreateDefaultSubobject<UGameItemContainerComponent>(TEXT("GameItemContainerComponent"));
}

TArray<UGameItemContainer*> AItemsDemoPlayerState::GetAllItemContainers() const
{
	return GameItemContainerComponent->GetAllItemContainers();
}

UGameItemContainer* AItemsDemoPlayerState::GetItemContainer(FGameplayTag ContainerId) const
{
	return GameItemContainerComponent->GetItemContainer(ContainerId);
}
