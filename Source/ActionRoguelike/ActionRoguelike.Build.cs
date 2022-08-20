// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ActionRoguelike : ModuleRules
{
	public ActionRoguelike(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		
		PublicIncludePaths.AddRange(
			new string[] {
				"ActionRoguelike"
			}
		);
		
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		
		PublicDependencyModuleNames.AddRange(new string[] { "AIModule", "GameplayTasks", "UMG" ,"GameplayTags"});
		
		PublicDependencyModuleNames.AddRange(new string[] { /*"DataTable"*/"DeveloperSettings" });
		
		
		
		
		// Modules   不加以下这一行 编译也可以过。 具体使用要看 有没有 互相引用 ， 单向依赖的情况
		//PublicDependencyModuleNames.AddRange(new string[] { "GASSample", "GASAbilityDemo" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
