// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MassCompanion : ModuleRules
{
	public MassCompanion(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"InputCore",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"MassEntity",
				"StructUtils",
				"MassCommon",
				"MassMovement",
				"MassActors",
				"MassSpawner",
				"MassGameplayDebug",
				"MassSignals",
				"MassCrowd",
				"MassActors",
				"MassSpawner",
				"MassRepresentation",
				"MassReplication",
				"MassNavigation",
				"MassSimulation",
				//needed for replication setup
				"NetCore",
				"AIModule",

				"ZoneGraph",
				"MassGameplayDebug",
				"MassZoneGraphNavigation", 
				"Niagara",
				"DeveloperSettings",
				"GeometryCore",
				"MassAIBehavior",
				"StateTreeModule",
				"MassLOD",
				"NavigationSystem",
				"Chaos",
				"PhysicsCore",
				"ChaosCore",
				"ChaosSolverEngine", "CADKernel",
				"RHI"
			}
		);
		
		//todo: maybe do thee editor only stuff on another module?

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("CodeView");
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
