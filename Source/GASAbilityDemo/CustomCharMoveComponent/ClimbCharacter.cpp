// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbCharacter.h"

#include "ClimbCMC.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AClimbCharacter::AClimbCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AClimbCharacter::BeginPlay()
{
	Super::BeginPlay();
  
}

// Called every frame
void AClimbCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AClimbCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}





void AClimbCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
    DOREPLIFETIME_CONDITION(AClimbCharacter, bIsClimbing, COND_SimulatedOnly);
    DOREPLIFETIME_CONDITION(AClimbCharacter, bIsClimbDashing, COND_SimulatedOnly);
}

void AClimbCharacter::StartClimbing()
{
    if (UClimbCMC* MovementComponent = Cast<UClimbCMC>(GetCharacterMovement()))
    {
        MovementComponent->bWantsToClimb = true;
    }
}

void AClimbCharacter::StopClimbing()
{
    if (UClimbCMC* MovementComponent = Cast<UClimbCMC>(GetCharacterMovement()))
    {
        MovementComponent->bWantsToClimb = false;
    }
}

bool AClimbCharacter::IsClimbingMoving()
{
    return bIsClimbing && GetVelocity().Size() > 0;
}

void AClimbCharacter::ClimbDash()
{
    if (UClimbCMC* MovementComponent = Cast<UClimbCMC>(GetCharacterMovement()))
    {
        MovementComponent->bWantsToClimbDash = true;
    }
}

void AClimbCharacter::OnRep_IsClimbing()
{
    UClimbCMC* MovementComponent = Cast<UClimbCMC>(GetCharacterMovement());
    if (bIsClimbing)
    {
        MovementComponent->bWantsToClimb = true;
    }else
    {
        MovementComponent->bWantsToClimb = false;
    }
}

void AClimbCharacter::OnRep_IsClimbDashing()
{
    UClimbCMC* MovementComponent = Cast<UClimbCMC>(GetCharacterMovement());
    if (bIsClimbDashing)
    {
        MovementComponent->bWantsToClimbDash = true;
    }else
    {
        MovementComponent->bWantsToClimbDash = false;
    }
}

void AClimbCharacter::OnStartClimb()
{
    
}

void AClimbCharacter::OnStopClimb()
{
    
}

void AClimbCharacter::EndClimbByJumpOver(const FVector& JumpOverLocation)
{
    const FVector MoveToLocation = JumpOverLocation + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 30);

    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    UKismetSystemLibrary::MoveComponentTo(Cast<USceneComponent>(GetCapsuleComponent()), MoveToLocation,
                                          GetActorRotation(), true, true, 0.8,
                                          true, EMoveComponentAction::Type::Move, LatentInfo);

    k2_EndClimbByJumpOver();
}

void AClimbCharacter::OnStartClimbDashing()
{
}

void AClimbCharacter::OnStopClimbDashing()
{
}

void AClimbCharacter::MoveForward(float Value)
{
    if (bIsClimbing)
    {
        const FVector Direction = FVector::CrossProduct(Cast<UClimbCMC>(GetCharacterMovement())->GetClimbSurfaceNormal(), -GetActorRightVector());
        AddMovementInput(Direction, Value);
    }
    else
    {
        AddMovementInput(
            UKismetMathLibrary::GetForwardVector(GetController()->GetControlRotation()),
            Value
        );
    }
}

void AClimbCharacter::MoveRight(float Value)
{
    if (bIsClimbing)
    {
        const FVector Direction = FVector::CrossProduct(Cast<UClimbCMC>(GetCharacterMovement())->GetClimbSurfaceNormal(), GetActorUpVector());
        AddMovementInput(Direction, Value);
    }
    else
    {
        AddMovementInput(
            UKismetMathLibrary::GetRightVector(GetController()->GetControlRotation()),
            Value
        );
    }
    
}

