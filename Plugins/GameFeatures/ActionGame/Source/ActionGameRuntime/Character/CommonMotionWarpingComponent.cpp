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
		TArray<FMotionWarpingTargetByLocationAndRotation> WarpTargetArray;

		for (auto Target : WarpTargets)
		{
			FMotionWarpingTargetByLocationAndRotation MotionWarpingTarget(Target.Name, Target.GetLocation(), Target.GetRotation());

			WarpTargetArray.Add(MotionWarpingTarget);
		}

		MulticastSyncWarpPoints(WarpTargetArray);
	}
}

void UCommonMotionWarpingComponent::MulticastSyncWarpPoints_Implementation(const TArray<FMotionWarpingTargetByLocationAndRotation>& Targets)
{
	for (const FMotionWarpingTargetByLocationAndRotation& Target : Targets)
	{
		AddOrUpdateWarpTargetFromLocationAndRotation(Target.Name, Target.Location, FRotator(Target.Rotation));
	}
}
