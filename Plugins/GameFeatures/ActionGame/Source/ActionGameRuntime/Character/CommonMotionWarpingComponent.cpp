// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonMotionWarpingComponent.h"


UCommonMotionWarpingComponent::UCommonMotionWarpingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//SetIsReplicated(true);
	
	// Can directly modify bReplicates,  without side effect.
	SetIsReplicatedByDefault(true);
}

void UCommonMotionWarpingComponent::SendWarpPointsToClients()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		// 5.0 Old version engine API.
		// if (GetOwnerRole() == ROLE_Authority)
		// {
		// 	TArray<FMotionWarpingTargetByLocationAndRotation> WarpTargets;
		//
		// 	for (auto WarpTarget : WarpTargetMap)
		// 	{
		// 		FMotionWarpingTargetByLocationAndRotation MotionWarpingTarget(WarpTarget.Key, WarpTarget.Value.GetLocation(), WarpTarget.Value.GetRotation());
		//
		// 		WarpTargets.Add(MotionWarpingTarget);
		// 	}
		//
		// 	MulticastSyncWarpPoints(WarpTargets);
		// }

		
		// 5.1 New engine API, TMap change to TArray.
		if (GetOwnerRole() == ROLE_Authority)
		{
			TArray<FMotionWarpingTargetByLocationAndRotation> WarpTargetArray;
	
			for (auto WarpTarget : WarpTargets)
			{
				FMotionWarpingTargetByLocationAndRotation MotionWarpingTarget(WarpTarget.Name, WarpTarget.Location, WarpTarget.Rotation.Quaternion());
	
				WarpTargetArray.Add(MotionWarpingTarget);
			}
	
			MulticastSyncWarpPoints(WarpTargetArray);
		}
	}
}

void UCommonMotionWarpingComponent::MulticastSyncWarpPoints_Implementation(const TArray<FMotionWarpingTargetByLocationAndRotation>& Targets)
{
	for (const FMotionWarpingTargetByLocationAndRotation& Target : Targets)
	{
		AddOrUpdateWarpTargetFromLocationAndRotation(Target.Name, Target.Location, FRotator(Target.Rotation));
	}
}
