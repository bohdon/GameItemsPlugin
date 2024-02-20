// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemDebugSubsystem.h"

#include "GameItemCheatsExtension.h"
#include "GameItemSettings.h"
#include "GameFramework/CheatManager.h"
#include "Misc/CoreDelegates.h"


void UGameItemDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FGameModeEvents::OnGameModePostLoginEvent().AddUObject(this, &UGameItemDebugSubsystem::OnPostLogin);
}

void UGameItemDebugSubsystem::Deinitialize()
{
	Super::Deinitialize();

	FGameModeEvents::OnGameModePostLoginEvent().RemoveAll(this);
}

void UGameItemDebugSubsystem::AddItemCheatsExtension(APlayerController* Player)
{
#if UE_WITH_CHEAT_MANAGER
	if (!Player || !Player->CheatManager)
	{
		return;
	}

	const TSoftClassPtr<UGameItemCheatsExtension> CheatManagerClassPtr = GetDefault<UGameItemSettings>()->ItemCheatsExtensionClass;
	if (CheatManagerClassPtr.IsNull())
	{
		return;
	}

	const UClass* CheatManagerClass = CheatManagerClassPtr.LoadSynchronous();
	UCheatManagerExtension* Extension = NewObject<UCheatManagerExtension>(Player->CheatManager, CheatManagerClass);
	SpawnedExtensions.Add(Extension);
	Player->CheatManager->AddCheatManagerExtension(Extension);
#endif
}

void UGameItemDebugSubsystem::RemoveItemCheatsExtension(APlayerController* Player)
{
#if UE_WITH_CHEAT_MANAGER
	if (!Player || !Player->CheatManager)
	{
		return;
	}

	// get all extensions for this cheat manager
	TArray<TWeakObjectPtr<UCheatManagerExtension>> PlayerExtensions = SpawnedExtensions.FilterByPredicate(
		[Player](const TWeakObjectPtr<UCheatManagerExtension>& Extension)
		{
			return Extension.IsValid() && Extension->GetOuterUCheatManager() == Player->CheatManager;
		});

	// remove extensions
	for (TWeakObjectPtr<UCheatManagerExtension> Extension : PlayerExtensions)
	{
		if (Extension.IsValid() && Extension->GetOuterUCheatManager() == Player->CheatManager)
		{
			Player->CheatManager->RemoveCheatManagerExtension(Extension.Get());
			SpawnedExtensions.Remove(Extension);
		}
	}
#endif
}

void UGameItemDebugSubsystem::OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPlayer)
{
	if (GameMode->AllowCheats(NewPlayer))
	{
		AddItemCheatsExtension(NewPlayer);
	}
}
