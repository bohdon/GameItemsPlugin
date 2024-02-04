// Copyright Bohdon Sayre, All Rights Reserved.

using UnrealBuildTool;

public class GameItemsPlugin : ModuleRules
{
	public GameItemsPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[]
		{
			// the game module doesnt use Public/Private folders,
			// add the module directory so it isn't required in every include path
			"$(ModuleDir)",
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"CommonUI",
			"Core",
			"CoreUObject",
			"Engine",
			"GameItems",
			"GameplayTags",
			"InputCore",
			"UMG",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}