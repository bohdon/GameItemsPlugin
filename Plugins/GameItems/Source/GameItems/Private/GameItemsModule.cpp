﻿// Copyright Bohdon Sayre, All Rights Reserved.

#include "GameItemsModule.h"

#include "Engine/Console.h"

#define LOCTEXT_NAMESPACE "FGameItemsModule"

DEFINE_LOG_CATEGORY(LogGameItems);

const FName ShowDebugNames::GameItems(TEXT("GameItems"));


void FGameItemsModule::StartupModule()
{
#if ALLOW_CONSOLE
	UConsole::RegisterConsoleAutoCompleteEntries.AddStatic(&FGameItemsModule::PopulateAutoCompleteEntries);
#endif // ALLOW_CONSOLE
}

void FGameItemsModule::ShutdownModule()
{
}

void FGameItemsModule::PopulateAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteList)
{
	const UConsoleSettings* ConsoleSettings = GetDefault<UConsoleSettings>();

	AutoCompleteList.AddDefaulted();

	FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.Last();
	AutoCompleteCommand.Command = FString::Printf(TEXT("ShowDebug %s"), *ShowDebugNames::GameItems.ToString());
	AutoCompleteCommand.Desc = TEXT("Toggles display of game item debug info");
	AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameItemsModule, GameItems)
