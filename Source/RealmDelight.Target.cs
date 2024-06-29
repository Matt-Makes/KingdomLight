// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class RealmDelightTarget : TargetRules
{
	public RealmDelightTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "ActionRoguelike" } );
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[]
		{
			"PredictedExample"
		});
		ExtraModuleNames.AddRange(new string[] {"Bong"});
		ExtraModuleNames.AddRange(new string[] {"GASAbilityDemo"});
		ExtraModuleNames.AddRange(new string[] {"GASSample"});
	}
}
