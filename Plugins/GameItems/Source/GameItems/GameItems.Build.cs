// Copyright Bohdon Sayre, All Rights Reserved.

using UnrealBuildTool;

public class GameItems : ModuleRules
{
	public GameItems(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"DeveloperSettings",
			"GameplayTags",
			"NetCore",
			"StructUtils",
			"WorldConditions",
		});


		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"EngineSettings",
			"Slate",
			"SlateCore",
		});
	}
}