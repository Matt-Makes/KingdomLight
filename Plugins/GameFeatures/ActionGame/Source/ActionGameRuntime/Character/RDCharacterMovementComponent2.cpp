// Fill out your copyright notice in the Description page of Project Settings.


#include "RDCharacterMovementComponent2.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"



void URDCharacterMovementComponent2::BeginPlay()
{
	Super::BeginPlay();

	HandleMovementDirection();
	
	
}

bool URDCharacterMovementComponent2::CanAttemptJump() const
{
	// Same as UCharacterMovementComponent's implementation but without the crouch check.
	return IsJumpAllowed() &&
		(IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}

void URDCharacterMovementComponent2::OnEnforcedStrafeTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	
}

EMovementDirectionType URDCharacterMovementComponent2::GetMovementDirectionType() const
{
	return EMovementDirectionType::None;
}

void URDCharacterMovementComponent2::SetMovementDirectionType(EMovementDirectionType InMovementDirectionType)
{
	
}

void URDCharacterMovementComponent2::HandleMovementDirection()
{
	switch (MovementDirectionType)
	{
	case EMovementDirectionType::Strafe:
		bUseControllerDesiredRotation = true;
		bOrientRotationToMovement = false;
		CharacterOwner->bUseControllerRotationYaw = true;
		break;
	default:
		bUseControllerDesiredRotation = false;
		bOrientRotationToMovement = true;
		CharacterOwner->bUseControllerRotationYaw = false;
		break;
	}
}

bool URDCharacterMovementComponent2::TryTraversal(UAbilitySystemComponent* ASC)
{
	return false;
}
