﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionGameCharacter.h"


#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"



#include "AbilitySystemComponent.h"
#include "CommonMotionWarpingComponent.h"
#include "ActionGameRuntime/AbilitySystem/Components/AG_AbilitySystemComponentBase.h"
//#include "GameplayEffectTypes.h"



AActionGameCharacter::AActionGameCharacter(const FObjectInitializer& ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CommonMotionWarpingComponent = CreateDefaultSubobject<UCommonMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
	
}

UAbilitySystemComponent* AActionGameCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AActionGameCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveEffectsWithTags(InAirTags);
	}
}

UCommonMotionWarpingComponent* AActionGameCharacter::GetCommonMotionWarpingComponent() const
{
	return CommonMotionWarpingComponent;
}

bool AActionGameCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext)
{
	return 0;
}

void AActionGameCharacter::InitializeAttributes()
{
}

void AActionGameCharacter::ApplyStartupEffects()
{
}

void AActionGameCharacter::GiveAbilities()
{
}

void AActionGameCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AActionGameCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}