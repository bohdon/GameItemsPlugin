// Copyright Bohdon Sayre, All Rights Reserved.

using UnrealBuildTool;

public class GameItemsPluginEditorTarget : TargetRules
{
	public GameItemsPluginEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

		ExtraModuleNames.AddRange(new string[]
		{
			"GameItemsPlugin",
		});
	}
}