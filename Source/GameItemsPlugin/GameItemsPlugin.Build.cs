// Copyright Bohdon Sayre, All Rights Reserved.

using UnrealBuildTool;

public class GameItemsPlugin : ModuleRules
{
	public GameItemsPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameItems",
			"InputCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}