// Copyright Bohdon Sayre, All Rights Reserved.

using UnrealBuildTool;

public class GameItems : ModuleRules
{
	public GameItems(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"DeveloperSettings",
			"GameplayTags",
			"NetCore",
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

		SetupGameplayDebuggerSupport(Target);
		SetupIrisSupport(Target);
	}
}