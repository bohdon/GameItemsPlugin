// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoPlayerState.h"

#include "GameItemContainerComponent.h"
#include "Demo/SaveGame/DemoPlayerSaveSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SaveGame.h"


AItemsDemoPlayerState::AItemsDemoPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameItemContainerComponent = CreateDefaultSubobject<UGameItemContainerComponent>(TEXT("GameItemContainerComponent"));
	GameItemContainerComponent->bEnableSaveGame = true;
	GameItemContainerComponent->bIsPlayerCollection = true;
	GameItemContainerComponent->SaveCollectionId = FName(TEXT("Player"));
}

void AItemsDemoPlayerState::ClientInitialize(AController* Controller)
{
	Super::ClientInitialize(Controller);

	// apply save game if it's already loaded
	if (const APlayerController* Player = Cast<APlayerController>(Controller))
	{
		if (const ULocalPlayer* LocalPlayer = Player->GetLocalPlayer())
		{
			const UDemoPlayerSaveSubsystem* SaveSubsystem = LocalPlayer->GetSubsystem<UDemoPlayerSaveSubsystem>();
			if (ULocalPlayerSaveGame* SaveGame = SaveSubsystem->GetSaveGame())
			{
				LoadSaveGame(SaveGame);
			}
		}
	}
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

void AItemsDemoPlayerState::CommitSaveGame(ULocalPlayerSaveGame* SaveGame) const
{
	GameItemContainerComponent->CommitSaveGame(SaveGame);
}

void AItemsDemoPlayerState::LoadSaveGame(ULocalPlayerSaveGame* SaveGame)
{
	GameItemContainerComponent->LoadSaveGame(SaveGame);
}
