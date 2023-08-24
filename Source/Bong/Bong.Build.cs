// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Bong : ModuleRules
{
	public Bong(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		
		PublicDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "GameplayTasks" });
		
		PublicDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemSteam" });
		
		PublicDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG" });
		
		PublicDependencyModuleNames.AddRange(new string[] { "GameFeatures",  "SignificanceManager", });
		
		PrivateDependencyModuleNames.AddRange(new string[] { "EnhancedInput"});
		
		PublicDependencyModuleNames.AddRange(new string[] { "Niagara" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
