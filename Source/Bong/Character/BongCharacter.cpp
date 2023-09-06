// Fill out your copyright notice in the Description page of Project Settings.


#include "BongCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Bong/Weapon/Weapon.h"
#include "Bong/BongComponents/CombatActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BongAnimInstance.h" // Runtime阶段才用到，所以没提示
#include "Bong/Bong.h"
#include "Bong/PlayerController/BongPlayerController.h"
#include "Bong/GameMode/BongGameMode.h"
#include "TimerManager.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Bong/PlayerState/BongPlayerState.h"
#include "Bong/Weapon/WeaponTypes.h"
// 定义 DOREPLIFETIME 的生命周期
#include "Net/UnrealNetwork.h"
// Enhanced Input
#include "EnhancedInputLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


class UEnhancedInputLocalPlayerSubsystem;
// Sets default values
ABongCharacter::ABongCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	Combat = CreateDefaultSubobject<UCombatActorComponent>(TEXT("CombatActorComp"));
	Combat->SetIsReplicated(true); // 组件无法脱离AActor单独存在，组件可以理解为AActor内的一个属性状态
	//Combat->AddToRoot();
	//SetRootComponent(BoxComp);
	
	// 四个端口， 三个生成器，有一些问题：重生过程中Character挤在一起，可能因为碰撞导致丢失掉一个玩家
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoomComp"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCameraComp"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	TurnRateGamepad = 45.f; // set our turn rates for input

	// 一般是未装备武器时的 设置
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f ,850.f, 0.f);
	
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidgetComp"));
	OverheadWidget->SetupAttachment(RootComponent);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; //设置CDO
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 解决人物卡摄像机
	
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh); // 角色的骨骼网格体设置成这个，检测跟着物理资产走
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); // 服务于HitTarget, 例如准星变红
	
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	// 解决客户端的各种卡顿问题
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimelineComp"));
}

// 确保先后顺序
void ABongCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (Combat)
	{
		Combat->BongCharacter = this; //赋予的一方作为友元类，访问私有变量并赋予，省去组件内的初始化
	}
}

// Enhanced Input
void ABongCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input 一开始 进行配置
	if ( APlayerController* PC = CastChecked<APlayerController>(GetController()) )
	{
		// 从Controller 里拿到Subsystem
		if(UEnhancedInputLocalPlayerSubsystem* EISubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			EISubsystem->AddMappingContext(IMC_Started, 0);
			EISubsystem->AddMappingContext(IMC_Triggered_MoveBase, 0);
		}
	}
	// 增强输入组件是其子类
	if(UEnhancedInputComponent* EIComponent = CastChecked<UEnhancedInputComponent, UInputComponent>(PlayerInputComponent))
	{
		// Triggered
		if(IA_MoveForward)
		{
			EIComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &ABongCharacter::MoveForward);
		}
		if(IA_MoveRight)
		{
			EIComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &ABongCharacter::MoveRight);
		}
		if(IA_Turn)
		{
			EIComponent->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &ABongCharacter::Turn);
		}
		if(IA_LookUp)
		{
			EIComponent->BindAction(IA_LookUp, ETriggerEvent::Triggered, this, &ABongCharacter::LookUp);
		}

		// Started
		if(IA_Jump)
		{
			EIComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ABongCharacter::Jump); //重写父类里的函数
			//EIComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ABongCharacter::StopJumping);
		}
		if(IA_Pickup_EquipWeapon)
		{
			EIComponent->BindAction(IA_Pickup_EquipWeapon, ETriggerEvent::Started, this, &ABongCharacter::EquipButtonPressed);
		}
		if(IA_Pickup_EquipWeapon)
		{
			EIComponent->BindAction(IA_Crouch, ETriggerEvent::Started, this, &ABongCharacter::CrouchButtonPressed);
		}
		if(IA_Reload)
		{
			EIComponent->BindAction(IA_Reload, ETriggerEvent::Started, this, &ABongCharacter::ReloadButtonPressed);
		}
		if(IA_Aim)
		{
			EIComponent->BindAction(IA_Aim, ETriggerEvent::Started, this, &ABongCharacter::AimButtonPressed);
			EIComponent->BindAction(IA_Aim, ETriggerEvent::Completed, this, &ABongCharacter::AimButtonReleased);
		}
		if(IA_Fire)
		{
			EIComponent->BindAction(IA_Fire, ETriggerEvent::Started, this, &ABongCharacter::FireButtonPressed);
			EIComponent->BindAction(IA_Fire, ETriggerEvent::Completed, this, &ABongCharacter::FireButtonReleased);
		}
	}
}

void ABongCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 初始化血条UI，只管客户端
	if(!HasAuthority() && IsLocallyControlled())
	{
		UpdateHUDHealth(); // 这里大概率是PC 的BeginPlay之后执行
	}

	// 服务端在这里不行，无法初始化，可能服务端太快了
	//UpdateHUDHealth();
	
	
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABongCharacter::ReceiveDamage);
	}
}

void ABongCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RotatingInPlace(DeltaTime);
	
	HideCameraIfCharacterClose();
	PollInit();
}

void ABongCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ABongCharacter, OverlappingWeapon, COND_OwnerOnly); //条件属性复制（只复制给拥有Owner Connection的），不看到别人显示的拾取UI
	DOREPLIFETIME(ABongCharacter, Health);
	DOREPLIFETIME(ABongCharacter, bDisableGameplay);
}


#pragma region Animation / AimOffset / Turning In Place replication / Dissolve Effect

#pragma endregion 


// ---------- 按键相关函数，思考 源头变量的网络复制，有些UE已做好复制处理，有些则没有 ---------- //
#pragma region Enhanced Input

/// Started
void ABongCharacter::Jump()
{
	if(bDisableGameplay) return;
	
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

// 玩家本地输入是OwnerOnly的，每个端口都有<此时就需要RPC了> 在多人情况下的武器拾取
// 捡起武器后需要 拾取UI的隐藏（利用枚举 EWeaponState进行Rep_Notify；再用枚举进行switch判断关闭 两端口UI，服务端关闭碰撞
void ABongCharacter::EquipButtonPressed()
{
	if(bDisableGameplay) return;
	
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon); //服务端就直接捡武器了
		}
		else
		{
			ServerEquipButtonPressed(); //客户端发RPC，Ownership 为Owned by invoking client
		}
	}
}

void ABongCharacter::CrouchButtonPressed()
{
	if(bDisableGameplay) return;
	
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch(); // 调用父类函数，改变相关变量，BongCharacter->bIsCrouched，动画蓝图每帧检测
	}
}

void ABongCharacter::ReloadButtonPressed()
{
	if(bDisableGameplay) return;
	
	if(Combat)
	{
		Combat->Reload();
	}
}

void ABongCharacter::AimButtonPressed()
{
	if(bDisableGameplay) return;
	
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABongCharacter::AimButtonReleased()
{
	if(bDisableGameplay) return;
	
	if(Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABongCharacter::FireButtonPressed()
{
	if(bDisableGameplay) return;
	
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABongCharacter::FireButtonReleased()
{
	if(bDisableGameplay) return;
	
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

/// Triggered
void ABongCharacter::MoveForward(const FInputActionValue& InputActionValueValue)
{
	if(Controller != nullptr)
	{
		if(bDisableGameplay) return;
        
        const float ForwardValue = InputActionValueValue.GetMagnitude(); // 增强输入，唯一特别的地方
        const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		if(ForwardValue != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, ForwardValue);
		}
	}
	// if(Controller != nullptr && Value != 0.f)
	// {
	// 	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	// 	// get forward vector
	// 	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); // 指定单位轴.X
	// 	AddMovementInput(Direction, Value);
	// }
}

void ABongCharacter::MoveRight(const FInputActionValue& InputActionValueValue)
{
	if(Controller != nullptr)
	{
		if(bDisableGameplay) return;
        
		const float RightValue = InputActionValueValue.GetMagnitude(); // 增强输入，唯一特别的地方
		const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		if(RightValue != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, RightValue);
		}
	}
	// if(bDisableGameplay) return;
	//
	// float Value = InputValue.GetMagnitude();
	// if(Controller != nullptr && Value != 0)
	// {
	// 	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	// 	// get right vector
	// 	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	// 	AddMovementInput(Direction, Value);
	// }
}

void ABongCharacter::Turn(const FInputActionValue& InputActionValueValue)
{
	if(Controller != nullptr)
	{
		const float TurnValue = InputActionValueValue.GetMagnitude();
		if(TurnValue != 0.0f)
		{
			AddControllerYawInput(TurnValue * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
		}
	}
	//AddControllerYawInput(InputValue.GetMagnitude());
}

void ABongCharacter::LookUp(const FInputActionValue& InputActionValueValue)
{
	if(Controller != nullptr)
	{
		const float LookUpValue = InputActionValueValue.GetMagnitude();
		if(LookUpValue != 0.0f)
		{
			AddControllerPitchInput(LookUpValue * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
		}
	}
	//AddControllerPitchInput(InputValue.GetMagnitude());
}
#pragma endregion


#pragma region Getters
bool ABongCharacter::IsEquippedWeapon()
{
	return (Combat && Combat->EquippedWeapon);
}

AWeapon* ABongCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

bool ABongCharacter::IsAiming()
{
	return (Combat && Combat->bCompAiming);
}

FVector ABongCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState ABongCharacter::GetCombatState() const
{
	if(Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState; // 战斗组件的职责
}

float ABongCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}
#pragma endregion 




// 在服务端执行
void ABongCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// BongGameMode = BongGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABongGameMode>() : BongGameMode;
	// if(BongGameMode == nullptr) return;
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	// 服务端执行的很好，客户端在血条被打空后，还要再挨一枪才行。原因是等到复制到 客户端时，这里已经执行完了
	if(Health <= 0.f)
	{
		//BongGameMode = GetWorld()->GetAuthGameMode<ABongGameMode>();
		if(ABongGameMode* GameMode = GetWorld()->GetAuthGameMode<ABongGameMode>())
		{
			ABongPlayerController* InstigatedByController = Cast<ABongPlayerController>(InstigatedBy); // 子类指针不能指向父类< 父类指针满足不了需求>，先把父类转换为子类
			
			BongPlayerController = BongPlayerController == nullptr ? Cast<ABongPlayerController>(Controller) : BongPlayerController;
			GameMode->PlayerEliminated(this, BongPlayerController, InstigatedByController);
		}
	}
}

void ABongCharacter::RotatingInPlace(float DeltaTime)
{
	if(bDisableGameplay)
	{
		// 这两个变量原来 是在左右移动镜头时，让人物保持原地不动的
		// bRotateRootBone = false;
		bUseControllerRotationYaw = false;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	// 上面就是自主代理和权威代理 <排除了服务器上的模拟代理>
	// 确保一定是每个端口上的 自主代理上执行
	if(GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime); // 每帧调用瞄准偏移<只在本地控制时使用> ； 模拟代理 想要瞄准偏移Yaw动画就必须每帧调用AimOffset()
	}
	else // 确保拿到所有模拟代理，包括服务端的
	{
		TimeSinceLastMovementReplication += DeltaTime; // 该变量达到一定的数量，代表已经很长时间没有复制移动了
		if(TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		
		// 模拟代理和自主代理都会每帧调用CalculateAO_Pitch()
		CalculateAO_Pitch();
	}
}

// 蒙太奇跟着动画蓝图走，后者又跟着Mesh走，Mesh在Character里
void ABongCharacter::PlayFireMontage(bool bAiming)
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && AM_FireWeapon)
	{
		AnimInstance->Montage_Play(AM_FireWeapon);
		FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABongCharacter::PlayReloadMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && AM_Reload)
	{
		AnimInstance->Montage_Play(AM_Reload);
		FName SectionName;
		switch(Combat->EquippedWeapon->GetWeaponType())/* 各自的职责分离，Character和战斗组件的相互引用 */
		{
		case EWeaponType::EWY_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
			
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWY_SMG:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWY_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWY_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABongCharacter::PlayHitReactMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && AM_HitReact)
	{
		AnimInstance->Montage_Play(AM_HitReact);
		FName SectionName("FromLeft");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABongCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && AM_Elim)
	{
		AnimInstance->Montage_Play(AM_Elim);
	}
}

/** GM只存在于服务端，服务端调用和多播调用 隔离开 */
void ABongCharacter::Eliminate()
{
	// 确保只在服务端调用丢 武器
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	
	MulticastEliminate();
	
	GetWorld()->GetTimerManager().SetTimer(ElimTimer, this, &ABongCharacter::ElimTimerFinished, ElimDelay);
}

void ABongCharacter::MulticastEliminate_Implementation()
{
	// 玩家被淘汰，UI武器枪膛内 子弹归0 
	if(BongPlayerController)
	{
		BongPlayerController->SetHUDWeaponAmmo(0);
	}
	
	bEliminated = true;
	
	PlayElimMontage();

	// Start dissolve effect been eliminating
	int32 MeshElementIndex = 0;
	for(UMaterialInstance* DissolveMaterialInstance : DissolveMaterialInstances)
	{
		// 如果蓝图已配置，且值传递有效
        if(DissolveMaterialInstance)
        {
            DynamicDissolveMaterialInstances[MeshElementIndex] = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
     
            // 给模型上材质，这一步很重要
            GetMesh()->SetMaterial(MeshElementIndex, DynamicDissolveMaterialInstances[MeshElementIndex]);

        	// 先给动态材质实例设置个 默认值
            DynamicDissolveMaterialInstances[MeshElementIndex]->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
            DynamicDissolveMaterialInstances[MeshElementIndex]->SetScalarParameterValue(TEXT("Glow"), 200.f);

        	++MeshElementIndex;
        }
	}
	StartDissolve();

	/** 这些只在消融后的一瞬间，在死亡位置的遗留有效，重生后恢复CDO */
	// Disable character movement been eliminating
	GetCharacterMovement()->DisableMovement(); // 会阻止重力对Character的影响
	GetCharacterMovement()->StopMovementImmediately(); // 禁止Character rotating, AI不需要输入也能移动
	if(BongPlayerController)
	{
		DisableInput(BongPlayerController);
	}

	// 禁止 玩家移动
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	// 避免 玩家已经消失，但枪还在开火
	if(Combat)
	{
		GetCombat()->FireButtonPressed(false);
	}
	
	// Disable collision(胶囊体和 骨骼网格体)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	
	// Spawn eliminate bot
	if(ElimBotVFX)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComp = UGameplayStatics::SpawnEmitterAtLocation(this, ElimBotVFX, ElimBotSpawnPoint, GetActorRotation());
	}
	if(ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}

	// 正在用狙击枪 瞄准的人被杀死时
	if(IsLocallyControlled() && Combat && Combat->bCompAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWY_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
}

// 该函数只在服务端调用
void ABongCharacter::ElimTimerFinished()
{
	ABongGameMode* TheBongGameMode = GetWorld()->GetAuthGameMode<ABongGameMode>();
	if(TheBongGameMode)
	{
		TheBongGameMode->RequestRespawn(this, Controller);
	}
}

void ABongCharacter::UpdateDissolveMaterialInstance(float DissolveValueOutput)
{
	for(UMaterialInstanceDynamic* DynamicDissolveMaterialInstance : DynamicDissolveMaterialInstances)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValueOutput);
	}
}

void ABongCharacter::StartDissolve()
{
	InterpFunc_DissolveTimelineTrack.BindDynamic(this, &ABongCharacter::UpdateDissolveMaterialInstance);
	
	if(FloatCurve_DissolveAsset && DissolveTimeline)
	{
		// Timeline在相应类型的轨道委托上使用曲线资产，回调会把曲线上读取的值输出
		DissolveTimeline->AddInterpFloat(FloatCurve_DissolveAsset, InterpFunc_DissolveTimelineTrack);
		DissolveTimeline->Play();
	}
}

// actual function implementation
// RPC函数，让客户端可以捡起武器 (本来是只有服务端才能捡起武器
void ABongCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		// 服务端捡起了武器，客户端镜像会及时更新
		Combat->EquipWeapon(OverlappingWeapon); //服务端运行，LocalRole == Role_Authority且 RemoteRole == Simulated Proxy
	}
}

/** 模拟代理的瞄准偏移Yaw动画需要优化，确保最小的性能开销；只有本地控制的Character可以执行AimOffset(floatDelta)；原先是所有Character函数都能执行此函数(服务器压力大），且功能都在这一个函数里 */
// 每帧调用，站立不动时只关注Yaw，Pitch什么都不需要关注
void ABongCharacter::AimOffset(float Delta)
{
	if(Combat && Combat->EquippedWeapon == nullptr) return; // 手拿武器才能瞄准偏移

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if(Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true; // 节省性能
		
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation); //例如：向右转，偏移动画需要一个正数，但这里小的减大的为负，所以需颠倒下
		AO_Yaw = DeltaAimRotation.Yaw;
		
		if(TurningInPlace == ETurningInPlace::ETIP_NotTurning) // 如果不转身
		{
			InterpAO_Yaw = AO_Yaw;
		}

		//// 站立不动时，相机让角色动，此时角色再使用Rotate Root Bone反方向Yaw移动也就相对是静止的; 反方向不是多次一举，方便之后AO_Yaw归 0, 即角色旋转至摄像机朝向
		//// @see OnRep_EquippedWeapon()
		bUseControllerRotationYaw = true;
		TurnInPlace(Delta);
	}
	if(Speed > 0.f || bIsInAir) // running or jumping
	{
		bRotateRootBone = false;
		
		// 移动时每帧缓存记录，用于站立时的Yaw计算
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABongCharacter::CalculateAO_Pitch()
{
	// GetBaseAimRotation()的返回值Rotation是一直 在两端同步的，但是发包时会压缩处理,范围变成了[0, 360)
	AO_Pitch = GetBaseAimRotation().Pitch;
	
	if(AO_Pitch > 90.f && !IsLocallyControlled()) // 非本地控制输入，即模拟时
	{
		// Map Pitch from (360, 270] to (0, -90]
		AO_Pitch = UKismetMathLibrary::MapRangeClamped(AO_Pitch, 360.f, 270.f, 0.f, -90.f);
	}
}

// 为了解决抖动，给模拟代理播放 原地转身动画，每当改变他的Controller Rotation时
void ABongCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	// Tick()已经确保在这里执行的 一定是模拟代理
	SimProxiesTurn();
	
	TimeSinceLastMovementReplication = 0.f;
}

//// 原则就是既然模拟代理的瞄准偏移Yaw动画已经确认放弃了，那就确保最小的性能开销，多一分也不值得
//// 只关心 阻止在模拟代理下的RotateRootBone为真，同时在人物移动时，其也设为false
void ABongCharacter::SimProxiesTurn()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false; // 模拟代理不使用根骨骼旋转，bUseControllerRotationYaw = true通过手动设置复制，模拟代理可以转身

	//bRotateRootBone = true; 设为true没有意义
	
	float Speed = CalculateSpeed();
	if(Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
 
	//UE_LOG(LogTemp, Warning, TEXT("ProxyYaw: %f"), ProxyYaw);

	// 要预防转的太慢的情况发生
	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
        {
        	TurningInPlace = ETurningInPlace::ETIP_Right;
        }
        else if(ProxyYaw < -TurnThreshold)
        {
        	TurningInPlace = ETurningInPlace::ETIP_Left;
        }
        else
        {
        	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
        }
		return;
	}
 
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABongCharacter::Destroyed()
{
	Super::Destroyed();
	
	if(ElimBotComp)
	{
		ElimBotComp->DestroyComponent();
	}
	
	BongGameMode = BongGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABongGameMode>() : BongGameMode;
	
	// 确保 在游戏过程中，不能销毁武器
	bool bNotMatchInProgress = BongGameMode && BongGameMode->GetMatchState() != MatchState::InProgress;
	if(Combat && Combat->EquippedWeapon && bNotMatchInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABongCharacter::TurnInPlace(float DeltaSeconds)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaSeconds, 4.f);
		AO_Yaw = InterpAO_Yaw; // AO_Yaw归 0, 即角色旋转至摄像机朝向
		if(FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 转身动画的停止条件
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABongCharacter::HideCameraIfCharacterClose()
{
	if(!IsLocallyControlled()) return;
	
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false); // 不能联网隐藏，其他玩家需要依然可以看见自己
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->SetOwnerNoSee(true); // 仅对Owner设置为 看不见
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->SetOwnerNoSee(false);
		}
	}
}

// 只在客户端执行，Health若没有更新，此处就不会执行
void ABongCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	
	// 使用服务端的变量复制， 再Rep_Notify 来替代 多播RPC
	PlayHitReactMontage();
}

void ABongCharacter::UpdateHUDHealth()
{
	// 防止两帧时间差的错传
	BongPlayerController = BongPlayerController == nullptr ? Cast<ABongPlayerController>(Controller) : BongPlayerController;
	
	// 职责分离：Character->Controller->HUD->创建WB_CharacterOverlay并添加到视口
	if(BongPlayerController)
	{
		BongPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABongCharacter::PollInit()
{
	if(BongPlayerState == nullptr)
	{
		BongPlayerState = GetPlayerState<ABongPlayerState>(); // 在Character的第一帧这里返回空
		if(BongPlayerState)
		{
			OnPlayerStateInitialized();
		}
	}
}

void ABongCharacter::OnPlayerStateInitialized()
{
	BongPlayerState->AddToScore(0.f);
	BongPlayerState->AddToDefeats(0);
}

/*
 * 因为UI 对每个端口都是独立的，和GI Input一样
 * 两个端口的 武器上的UI显示逻辑各自独立，无联系；交叠中的武器 这个变量也要和UI类似，需要是Condition为 OwnerOnly
 */
// AWeapon类在服务端调用 该函数
void ABongCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	/*
	 *服务端客户端通用逻辑，每次都事先设为false
	 */
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	// 指定Cond_OwnerOnly的变量复制时，若此时是服务器上的Server-owned actor，那就不会复制给客户端；所以C++客户端 Rep_Notify不会执行
	OverlappingWeapon = Weapon;

	
	/*
	 *服务端专有逻辑
	 */
	// 在服务端上的众多Controller，也是Authority里找到Server-owner actor，就是确保是主办人
	if (IsLocallyControlled())
	{
		/** 这里的判断非常关键，为的就是服务端的关UI，结束交叠时变量已设为空，设不了true，但之前已经设为false */
		// 服务端结束交叠时把变量设为空
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

// Rep_Notify只在客户端运行 C++版；且变量开启复制后才会调用
void ABongCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// 服务端有可能把变量设为空
	// 开始交叠：即第一次交叠，这里执行判断（结束交叠时这个变量会设为空
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	// 结束交叠：即第二次交叠、复制时 这个形参会在接受复制前缓存变量值，此时不为空（第一次复制前形参为空
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}





