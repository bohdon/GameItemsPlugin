// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSettings.h"

#include "GameItemsModule.h"


FName UGameItemSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

FGameplayTag UGameItemSettings::GetDefaultContainerId()
{
	const UGameItemSettings* Settings = GetDefault<UGameItemSettings>();
	if (Settings->bRequireValidDefaultContainerId && !Settings->DefaultContainerId.IsValid())
	{
		ensureMsgf(false, TEXT("UGameItemSettings.DefaultContainerId is not set. "
			           "Set a value or disable `bRequireValidDefaultContainerId` in the project settings"));
	}
	return Settings->DefaultContainerId;
}
