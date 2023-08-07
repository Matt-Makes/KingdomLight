// Fill out your copyright notice in the Description page of Project Settings.


#include "RDCharacter.h"

//#include "RDCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ARDCharacter::ARDCharacter()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARDCharacter::BeginPlay()
{
    Super::BeginPlay();
    
}

// Called every frame
void ARDCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ARDCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

}






void ARDCharacter::Jump()
{
    bPressedRDJump = true;
    Super::Jump();

    // Prevent character doing the default Jump before run Mantle code,then set it back to true and let the Jump run as it intended.
    bPressedJump = false; // lock CheckJumpInput().
	
    //LogOnScreen(GetWorld(), TEXT("Jumppppppingggg!!!!!"), FColor::Green);
}

void ARDCharacter::StopJumping()
{
    //bClimbing ? bClimbing = false : bClimbing = true;
    bPressedRDJump = false;
	
    //UE_LOG(LogTemp, Error, TEXT("FFFunction %s, call StopJumping()222"), *FString(__FUNCTION__));
    Super::StopJumping();

    //LogOnScreen(GetWorld(), TEXT("Stop Jumppppppingggg!!!!!"), FColor::Green);
}







// Called by custom CMC.
FCollisionQueryParams ARDCharacter::GetIgnoreCharacterParams() const
{
    FCollisionQueryParams Params;

    TArray<AActor*> CharacterChild;
    GetAllChildActors(CharacterChild);
    Params.AddIgnoredActors(CharacterChild); // Ignore this character and all its children.
    Params.AddIgnoredActor(this);
	
    return Params;
}

//
// void ARDCharacter::MoveForward(float Value)
// {
//     if (bClimbing)
//     {
//         const FVector Direction = FVector::CrossProduct(Cast<URDCharacterMovementComponent>(GetCharacterMovement())->GetClimbSurfaceNormal(), -GetActorRightVector());
//         AddMovementInput(Direction, Value);
//     }
//     else
//     {
//         AddMovementInput(
//                 UKismetMathLibrary::GetForwardVector(GetController()->GetControlRotation()),
//                 Value
//         );
//     }
// }
//
// void ARDCharacter::MoveRight(float Value)
// {
//     if (bClimbing)
//     {
//         const FVector Direction = FVector::CrossProduct(Cast<URDCharacterMovementComponent>(GetCharacterMovement())->GetClimbSurfaceNormal(), GetActorUpVector());
//         AddMovementInput(Direction, 0.00f);
//     }
//     else
//     {
//         AddMovementInput(
//                 UKismetMathLibrary::GetRightVector(GetController()->GetControlRotation()),
//                 Value
//         );
//     }
// }
