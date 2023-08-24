// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionWarpingComponent.h"
#include "ActionGameRuntime/ActionGameTypes.h"
#include "CommonMotionWarpingComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ACTIONGAMERUNTIME_API UCommonMotionWarpingComponent : public UMotionWarpingComponent
{
	GENERATED_BODY()

public:
	UCommonMotionWarpingComponent(const FObjectInitializer& ObjectInitializer);

	void SendWarpPointsToClients();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSyncWarpPoints(const TArray<FMotionWarpingTargetByLocationAndRotation>& Targets);
};
