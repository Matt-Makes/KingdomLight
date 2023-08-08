// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EMovementDirectionType : uint8
{
	None  UMETA(DisplayName = "None"),
	OrientToMovement  UMETA(DisplayName = "OrientToMovement"),
	Strafe  UMETA(DisplayName = "Strafe")
};

USTRUCT(BlueprintType)
struct FMotionWarpingTargetByLocationAndRotation
{
	GENERATED_USTRUCT_BODY();

	FMotionWarpingTargetByLocationAndRotation()
	{

	}

	FMotionWarpingTargetByLocationAndRotation(FName InName, FVector InLocation, FQuat InRotation)
		: Name(InName)
		, Location(InLocation)
		, Rotation(InRotation)
	{

	}

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FQuat Rotation;
};
