using UnrealBuildTool;

public class GASAbilityDemo : ModuleRules
{
    public GASAbilityDemo(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                "GASAbilityDemo",
                
                
                "ActionRoguelike"
            }
        );
        
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                
                "HeadMountedDisplay" /* 没有还报错 */
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            }
        );
        
        PrivateDependencyModuleNames.AddRange(new string[] {"GameplayTags", "GameplayAbilities", "GameplayTasks"});
    }
}