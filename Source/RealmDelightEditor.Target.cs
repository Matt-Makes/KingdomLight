// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class RealmDelightEditorTarget : TargetRules
{
	public RealmDelightEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
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
