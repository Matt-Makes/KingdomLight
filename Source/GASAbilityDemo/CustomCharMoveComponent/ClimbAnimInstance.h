// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ClimbAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GASABILITYDEMO_API UClimbAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UClimbAnimInstance() :bIsClimbing(0), ClimbVelocity2D(0.f), ClimbDashVelocity2D(0.f), bIsClimbDashing(0) {}

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationState|Climb")
	uint8 bIsClimbing : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationState|Climb")
	FVector2D ClimbVelocity2D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationState|Climb")
	FVector2D ClimbDashVelocity2D;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationState|Climb")
	uint8 bIsClimbDashing : 1;

	virtual void NativeUpdateAnimation(float DeltaTime) override;
};