// Copyright Bohdon Sayre, All Rights Reserved.

using UnrealBuildTool;

public class GameItemsPluginTarget : TargetRules
{
	public GameItemsPluginTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		ExtraModuleNames.AddRange(new string[]
		{
			"GameItemsPlugin"
		});
	}
}