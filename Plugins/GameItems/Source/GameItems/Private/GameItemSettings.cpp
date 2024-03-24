// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSettings.h"

#include "GameItemCheatsExtension.h"
#include "GameItemDebugSubsystem.h"
#include "GameItemsModule.h"


UGameItemSettings::UGameItemSettings()
	: bRequireValidDefaultContainerId(true),
	  ItemCheatsExtensionClass(UGameItemCheatsExtension::StaticClass())
{
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
