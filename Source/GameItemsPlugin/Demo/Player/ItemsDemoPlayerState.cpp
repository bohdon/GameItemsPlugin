// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoPlayerState.h"

#include "GameItemContainerComponent.h"


AItemsDemoPlayerState::AItemsDemoPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameItemContainerComponent = CreateDefaultSubobject<UGameItemContainerComponent>(TEXT("GameItemContainerComponent"));
	GameItemContainerComponent->bEnableSaveGame = true;
	GameItemContainerComponent->bIsPlayerCollection = true;
	GameItemContainerComponent->SaveCollectionId = FName(TEXT("Player"));
}

TArray<UGameItemContainer*> AItemsDemoPlayerState::GetAllItemContainers() const
{
	return GameItemContainerComponent->GetAllItemContainers();
}

UGameItemContainer* AItemsDemoPlayerState::GetItemContainer(FGameplayTag ContainerId) const
{
	return GameItemContainerComponent->GetItemContainer(ContainerId);
}

UGameItemContainerComponent* AItemsDemoPlayerState::GetItemContainerComponent() const
{
	return GameItemContainerComponent;
}

void AItemsDemoPlayerState::CommitSaveGame(USaveGame* SaveGame)
{
	GameItemContainerComponent->CommitSaveGame(SaveGame);
}

void AItemsDemoPlayerState::LoadSaveGame(USaveGame* SaveGame)
{
	GameItemContainerComponent->LoadSaveGame(SaveGame);
}
