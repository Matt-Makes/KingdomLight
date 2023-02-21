// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCharMoveComponent/ClimbAnimInstance.h"

#include "ClimbCharacter.h"
#include "ClimbCMC.h"
#include "Kismet/KismetMathLibrary.h"


void UClimbAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (const AClimbCharacter* CharacterBase = Cast<AClimbCharacter>(GetOwningActor()))
	{
		bIsClimbing = CharacterBase->bIsClimbing;

		if (bIsClimbing)
		{
			const FVector Rotate = UKismetMathLibrary::LessLess_VectorRotator(GetOwningActor()->GetVelocity(), CharacterBase->GetActorRotation());
			ClimbVelocity2D = FVector2D(Rotate.Y, Rotate.Z);
		}
		
		bIsClimbDashing = CharacterBase->bIsClimbDashing;
		if (bIsClimbDashing)
		{
			if (UClimbCMC* MovementComponent = Cast<UClimbCMC>(CharacterBase->GetMovementComponent()))
			{
				const FVector ClimbDashRotate = UKismetMathLibrary::LessLess_VectorRotator(MovementComponent->GetClimbDashDirection(), CharacterBase->GetActorRotation());
				ClimbDashVelocity2D =  FVector2D(ClimbDashRotate.Y, ClimbDashRotate.Z);
			}
		}
	}
}

