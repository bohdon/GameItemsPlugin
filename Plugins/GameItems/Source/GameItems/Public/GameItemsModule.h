// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FAutoCompleteCommand;

GAMEITEMS_API DECLARE_LOG_CATEGORY_EXTERN(LogGameItems, Log, All);

namespace ShowDebugNames
{
	/** ShowDebug name for displaying game item debug info. */
	extern GAMEITEMS_API const FName GameItems;
};


class FGameItemsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#if ALLOW_CONSOLE
	/* Callback function registered with Console to inject show debug auto complete command */
	static void PopulateAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteList);
#endif
};
