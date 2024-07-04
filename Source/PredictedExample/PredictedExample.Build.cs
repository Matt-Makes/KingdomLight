using UnrealBuildTool;

public class PredictedExample : ModuleRules
{
    public PredictedExample(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        {
	        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

	        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput", "PredictedMovement" });
        }
    }
}