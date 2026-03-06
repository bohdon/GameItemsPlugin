// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSettings.h"

#include "GameItemCheatsExtension.h"
#include "GameItemsModule.h"


UGameItemSettings::UGameItemSettings()
	: bRequireValidDefaultContainerId(true)
	, ItemCheatsExtensionClass(UGameItemCheatsExtension::StaticClass())
{
#if UE_WITH_CHEAT_MANAGER
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(
			[](UCheatManager* CheatManager)
				{
					const TSoftClassPtr<UGameItemCheatsExtension>& CheatManagerClassPtr = GetDefault<UGameItemSettings>()->ItemCheatsExtensionClass;
					if (CheatManagerClassPtr.IsNull())
					{
						return;
					}

					const UClass* CheatManagerClass = CheatManagerClassPtr.LoadSynchronous();
					CheatManager->AddCheatManagerExtension(NewObject<UGameItemCheatsExtension>(CheatManager, CheatManagerClass));
				}));
	}
#endif
}

FName UGameItemSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FGameplayTag UGameItemSettings::GetDefaultContainerId()
{
	const UGameItemSettings* Settings = GetDefault<UGameItemSettings>();
	if (Settings->bRequireValidDefaultContainerId && !Settings->DefaultContainerId.IsValid())
	{
		UE_LOG(LogGameItems, Error, TEXT("UGameItemSettings.DefaultContainerId is not set. "
			"Set a value or disable `bRequireValidDefaultContainerId` in the project settings"));
	}
	return Settings->DefaultContainerId;
}
