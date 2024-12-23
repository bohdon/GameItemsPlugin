﻿// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameplayAbilityItems : ModuleRules
{
	public GameplayAbilityItems(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"ExtendedGameplayAbilities",
			"GameItems",
			"GameplayAbilities",
			"GameplayTags",
		});


		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
		});
	}
}