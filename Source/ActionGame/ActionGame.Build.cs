using UnrealBuildTool;

public class ActionGame : ModuleRules
{
    public ActionGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                /*"LyraGame",*/
                "ModularGameplay",
                /*"CommonGame",*/
                // ... add other public dependencies that you statically link with here ...
            }
            );
			
		
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "GameplayTags",
                "GameplayTasks",
                "GameplayAbilities",
                /*"GameplayMessageRuntime",*/
                "CommonUI",
                "UMG",
                "DataRegistry",
                /*"AsyncMixin",*/
                "EnhancedInput",
                /*"GameSubtitles",*/
                "DeveloperSettings",
                "AIModule", 
                "MotionWarping"
                // ... add private dependencies that you statically link with here ...	
            }
            );
		
		
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );
    }
}