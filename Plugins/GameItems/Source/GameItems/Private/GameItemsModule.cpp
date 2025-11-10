// Copyright Bohdon Sayre, All Rights Reserved.

#include "GameItemsModule.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "GameplayDebuggerCategory_GameItems.h"
#endif

#include "Engine/Console.h"

#define LOCTEXT_NAMESPACE "FGameItemsModule"

DEFINE_LOG_CATEGORY(LogGameItems);

const FName ShowDebugNames::GameItems(TEXT("GameItems"));


void FGameItemsModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory(
		"GameItems",
		IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_GameItems::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGame, 7);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif

#if ALLOW_CONSOLE
	UConsole::RegisterConsoleAutoCompleteEntries.AddStatic(&FGameItemsModule::PopulateAutoCompleteEntries);
#endif
}

void FGameItemsModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory("GameItems");
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
#endif
}

#if ALLOW_CONSOLE
void FGameItemsModule::PopulateAutoCompleteEntries(TArray<FAutoCompleteCommand>& AutoCompleteList)
{
	const UConsoleSettings* ConsoleSettings = GetDefault<UConsoleSettings>();

	AutoCompleteList.AddDefaulted();

	FAutoCompleteCommand& AutoCompleteCommand = AutoCompleteList.Last();
	AutoCompleteCommand.Command = FString::Printf(TEXT("ShowDebug %s"), *ShowDebugNames::GameItems.ToString());
	AutoCompleteCommand.Desc = TEXT("Toggles display of game item debug info");
	AutoCompleteCommand.Color = ConsoleSettings->AutoCompleteCommandColor;
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameItemsModule, GameItems)
