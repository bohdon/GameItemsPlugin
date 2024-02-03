// Copyright Bohdon Sayre, All Rights Reserved.

using UnrealBuildTool;

public class GameItemsPluginTarget : TargetRules
{
	public GameItemsPluginTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

		ExtraModuleNames.AddRange(new string[]
		{
			"GameItemsPlugin"
		});
	}
}