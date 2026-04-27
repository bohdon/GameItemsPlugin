// Copyright Bohdon Sayre, All Rights Reserved.


#include "ItemsDemoPlayerState.h"

#include "GameItemContainer.h"
#include "GameItemContainerComponent.h"
#include "TimerManager.h"
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
			SaveSubsystem = LocalPlayer->GetSubsystem<UDemoPlayerSaveSubsystem>();
			SaveSubsystem->OnSaveGameChangedEvent.AddUObject(this, &ThisClass::OnSaveGameChanged);
			SaveSubsystem->OnCommitSaveGameEvent.AddUObject(this, &ThisClass::OnCommitSaveGame);

			TryApplySaveGame();
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

void AItemsDemoPlayerState::OnCommitSaveGame(ULocalPlayerSaveGame* SaveGame) const
{
	GameItemContainerComponent->CommitSaveGame(SaveGame);
}

void AItemsDemoPlayerState::OnSaveGameChanged()
{
	TryApplySaveGame();
}

bool AItemsDemoPlayerState::IsReadyToApplySaveGame() const
{
	if (!GetPlayerController())
	{
		return false;
	}

	// don't load save data until the container and its rules are all replicated.

	// even though player inventory may be local only, container graphs and rules are still setup by the server,
	// so they can communicate with the server when necessary, usually for item transfer.

	// TODO: add a more generic way to check?
	for (const UGameItemContainer* Container : GameItemContainerComponent->GetAllItemContainers())
	{
		if (!Container)
		{
			// null entry means not replicated yet
			return false;
		}

		if (Container->GetRules().Contains(nullptr))
		{
			// null entry means not replicated yet
			return false;
		}
	}

	return true;
}

void AItemsDemoPlayerState::TryApplySaveGame()
{
	ULocalPlayerSaveGame* SaveGame = SaveSubsystem->GetSaveGame();
	if (!SaveGame)
	{
		return;
	}

	if (!IsReadyToApplySaveGame())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::TryApplySaveGame);
		return;
	}

	GameItemContainerComponent->LoadSaveGame(SaveGame);
}
