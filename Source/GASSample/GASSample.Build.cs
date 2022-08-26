// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
public class GASSample : ModuleRules
{
	public GASSample(ReadOnlyTargetRules Target)
		: base(Target)
	{
		// PrivatePCHHeaderFile = "Public/XEnergyCollect.h";
		// PCHUsage = PCHUsageMode.UseSharedPCHs;
		
		//PrivateIncludePaths.Add("XEnergyCollect/Private");
		
		PublicIncludePaths.AddRange(
			new string[] {
				"GASSample",
				
				"ActionRoguelike"
			}
		);
		
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"Niagara"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"MoviePlayer",
				"Slate",
				"SlateCore",
				"InputCore"
				
			}
		);
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", 
			"GameplayAbilities",/* 修改：注册插件 */
			"GameplayTags", "GameplayTasks"/* 修改：自定义Task需要 */
		});
	}
}
