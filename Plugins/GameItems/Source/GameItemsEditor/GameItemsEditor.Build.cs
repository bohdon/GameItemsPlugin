// Copyright Bohdon Sayre, All Rights Reserved.

using UnrealBuildTool;

public class GameItemsEditor : ModuleRules
{
	public GameItemsEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"AssetDefinition",
			"Core",
			"CoreUObject",
			"EditorSubsystem",
			"Engine",
			"EngineAssetDefinitions",
			"GameItems",
			"UMG",
			"UnrealEd",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AssetTools",
			"ContentBrowser",
			"RenderCore",
			"Slate",
			"SlateCore",
			"ToolMenus",
		});
	}
}