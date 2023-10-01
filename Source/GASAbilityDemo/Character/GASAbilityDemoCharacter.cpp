// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASAbilityDemoCharacter.h"
//#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystemComponent.h"
#include "GASAbilityDemo/AttributeSet/GASAbilityDemoAttributeSet.h"


//////////////////////////////////////////////////////////////////////////
// AGASAbilityDemoCharacter

AGASAbilityDemoCharacter::AGASAbilityDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	//Add the Ability System Component to the character.
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("Ability System Component"));

}

void AGASAbilityDemoCharacter::CancelAbilityWithTags(const FGameplayTagContainer CancelWithTags)
{
	//The Ability System Component has multiple ways of cancelling and activating abilities. Using tags provides a generic means of categorizing different types of abilities without knowing what specific ability is active.
	AbilitySystemComponent->CancelAbilities(&CancelWithTags);
}

void AGASAbilityDemoCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Initializes the attribute set using a data table.
	if (IsValid(AbilitySystemComponent))
	{
		AttributeSet = AbilitySystemComponent->GetSet<UGASAbilityDemoAttributeSet>();

		//GetGameplayAttributeValueChangedDelegate will enable you to bind delegates without programming them manually.
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &AGASAbilityDemoCharacter::OnHealthChangedInternal);
	}
}

void AGASAbilityDemoCharacter::OnHealthChangedInternal(const FOnAttributeChangeData & Data)
{
	OnHealthChanged(Data.OldValue, Data.NewValue);
}

//Getters for Health, Max Health, Strength, Endurance, and Base Damage. All of these have bail-out logic for if the AttributeSet isn't initialized correctly.
float AGASAbilityDemoCharacter::GetHealth() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetHealth();
	}

	return -1.0f;
}

float AGASAbilityDemoCharacter::GetMaxHealth() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetMaxHealth();
	}

	return 0.0f;
}

float AGASAbilityDemoCharacter::GetStrength() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetStrength();
	}

	return 0.0f;
}

float AGASAbilityDemoCharacter::GetEndurance() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetEndurance();
	}

	return 0.0f;
}

float AGASAbilityDemoCharacter::GetBaseDamage() const
{
	if (IsValid(AttributeSet))
	{
		return AttributeSet->GetWeaponDamage();
	}

	return 0.0f;
}

//Create an instance of an Ability and add it to the Ability System Component.
void AGASAbilityDemoCharacter::GrantAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level, int32 InputCode)
{
	if (GetLocalRole() == ROLE_Authority && IsValid(AbilitySystemComponent) && IsValid(AbilityClass))
	{
		// 获取CDO的方式
		UGameplayAbility* Ability = AbilityClass->GetDefaultObject<UGameplayAbility>();

		if (IsValid(Ability))
		{
			// 含 元数据 的结构体，包含了引用计数
			// create the new ability spec struct. Ability specs contain metadata about the ability, like what level they're set to, as well as a reference to the ability.
			FGameplayAbilitySpec AbilitySpec(
				Ability,
				Level,
				InputCode
			);

			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}


	// ------------------ 更直观，不方便控制GA的初始等级 -------------------- //

	// 将在游戏启动时被赋予角色的Abilities数组
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	// TArray<TSubclassOf<class UGameplayAbility>> PreloadedAbilities;
	
	// Super::BeginPlay();
	// if (AbilitySystemComponent != nullptr)
	// {
	// 	//初始化技能
	// 	if (PreloadedAbilities.Num() > 0)
	// 	{
	// 		for (auto i = 0; i < PreloadedAbilities.Num(); i++)
	// 		{
	// 			if (PreloadedAbilities[i] != nullptr)
	// 			{
	// 				// FGameplayAbilitySpec是GA的实例，其构造函数的第二个参数代表GA的等级，这里暂令其全部为1
	// 				AbilitySystemComponent->GiveAbility(
	// 				   FGameplayAbilitySpec(PreloadedAbilities[i].GetDefaultObject(), 1));
	// 			}
	// 		}
	// 	}
	// }

	//初始化ASC
	//AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
}

void AGASAbilityDemoCharacter::ActivateAbility(int32 InputCode)
{
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->AbilityLocalInputPressed(InputCode);
	}
}


#pragma region


//////////////////////////////////////////////////////////////////////////
// Input

void AGASAbilityDemoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGASAbilityDemoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGASAbilityDemoCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGASAbilityDemoCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGASAbilityDemoCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AGASAbilityDemoCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AGASAbilityDemoCharacter::TouchStopped);

	// VR headset functionality
	//PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AGASAbilityDemoCharacter::OnResetVR);
}



// void AGASAbilityDemoCharacter::OnResetVR()
// {
// 	// If GASAbilityDemo is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in GASAbilityDemo.Build.cs is not automatically propagated
// 	// and a linker error will result.
// 	// You will need to either:
// 	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
// 	// or:
// 	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
// 	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
// }

void AGASAbilityDemoCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AGASAbilityDemoCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AGASAbilityDemoCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGASAbilityDemoCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AGASAbilityDemoCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AGASAbilityDemoCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


#pragma endregion 

