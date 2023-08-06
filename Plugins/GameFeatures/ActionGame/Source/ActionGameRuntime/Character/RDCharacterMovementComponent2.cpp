// Fill out your copyright notice in the Description page of Project Settings.


#include "RDCharacterMovementComponent2.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"QA	

bool URDCharacterMovementComponent2::TryTraversal(UAbilitySystemComponent* ASC)
{
}

void URDCharacterMovementComponent2::BeginPlay()
{
	Super::BeginPlay();
}

EMovementDirectionType URDCharacterMovementComponent2::GetMovementDirectionType() const
{
}

void URDCharacterMovementComponent2::SetMovementDirectionType(EMovementDirectionType InMovementDirectionType)
{
}

void URDCharacterMovementComponent2::OnEnforcedStrafeTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
}

void URDCharacterMovementComponent2::HandleMovementDirection()
{
}
