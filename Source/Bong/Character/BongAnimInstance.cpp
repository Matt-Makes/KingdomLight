// Fill out your copyright notice in the Description page of Project Settings.


#include "BongAnimInstance.h"
#include "BongCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Bong/Weapon/Weapon.h"
#include "Bong/BongTypes/CombatState.h"


UBongAnimInstance::UBongAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBongAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BongCharacter = Cast<ABongCharacter>(TryGetPawnOwner());
}

void UBongAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(BongCharacter == nullptr)
	{
		BongCharacter = Cast<ABongCharacter>(TryGetPawnOwner());
	}
	if(BongCharacter == nullptr) return; //多重检查

	FVector Velocity = BongCharacter->GetVelocity();
	Velocity.Z = 0;
	Speed = Velocity.Size();
	//Speed = Velocity.Length();

	// MovementComp
	bIsInAir = BongCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BongCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	
	/** 每帧设置这些bool变量，源头是可复制变量 */
	bEquippedWeapon = BongCharacter->IsEquippedWeapon();
	EquippedWeapon = BongCharacter->GetEquippedWeapon();
	bCrouched = BongCharacter->bIsCrouched; //UE已经执行好RepNotify   Set by character movement to specify that this Character is currently crouched
	bAiming = BongCharacter->IsAiming();
	TurningInPlace = BongCharacter->GetTurningInPlace();
	bRotateRootBone = BongCharacter->ShouldRotateRootBone();
	bEliminated = BongCharacter->IsEliminated();
	bUseFABRIK = BongCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	
	bUseAimOffsets = BongCharacter->GetCombatState() != ECombatState::ECS_Reloading && !BongCharacter->GetDisableGameplay();
	bModifyRightHand = BongCharacter->GetCombatState() != ECombatState::ECS_Reloading && !BongCharacter->GetDisableGameplay();
	
	
	/** Offset Yaw for Strafing */
	FRotator AimRotation = BongCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BongCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	// 该函数自带最短路径<-180直接到180>；不能插值float，会从-180到 0 再到180，数字是这么一个规律
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	// 向右0到180，向左0到-180(因为要获取速度，所以角色动起来，这两个全局变量相减才有意义；两个全局变量都会变，但是正因为全局，相减时的值代表了某种状态)
	YawOffset = DeltaRotation.Yaw;
	//UE_LOG(LogTemp, Warning, TEXT("YawOffset: %f"), YawOffset);

	
	/** Leaning calculate for character */
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BongCharacter->GetActorRotation(); //胶囊体组件的Rotation
	// Delta很小，两帧之间的值需要scale up；一般是乘以 <这里除以是确保两端口的计算结果一致>
	const FRotator DeltaCharacterRotator = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = DeltaCharacterRotator.Yaw / DeltaSeconds; // Yaw越大除以的 就越多
	
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f); //InterpSpeed直接决定快慢，代表了每秒多少个Unit，不用管帧数高低
	/** 两个90是混合空间里的垂直轴的坐标，可以更改修改偏移角度的大小 */
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	

	/** 瞄准偏移 */
	AO_Yaw = BongCharacter->GetAO_Yaw();
	AO_Pitch = BongCharacter->GetAO_Pitch();

	// 计算每把枪的 左手插槽 合适位置，之后在蓝图 用于FaBIK
	if(bEquippedWeapon && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BongCharacter->GetMesh() )
	{
		// Get SocketTransform in world space(世界空间更准确.
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Socket_LeftHand"), ERelativeTransformSpace::RTS_World);
		// Set our left hand socket on the weapon in world space Relative to our right hand in bone space.
		FVector OutLocation;
		FRotator OutRotation;
		BongCharacter->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutLocation, OutRotation);
		LeftHandTransform.SetLocation(OutLocation);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		// 虽然动画蓝图在每一台机器Separate 的运行，但只有Autonomous代理才有必要执行这些逻辑。 模拟代理的 武器指向和瞄准方向一致并不重要，为节省服务器带宽，忽略掉(且HitTarget没有复制，不会有来自服务器的源头变量权威复制，为节省服务器性能，只给自主代理的动画蓝图使用，模拟代理的不用)
		if(BongCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = BongCharacter->GetMesh()->GetSocketTransform(FName("RightHand"), ERelativeTransformSpace::RTS_World);
			//FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("RightHand"), ERelativeTransformSpace::RTS_World);
			//RightHandTransform.GetRotation().Rotator() = RightHandTransform.GetRotation().Rotator() + FRotator(0.f, 0.f, -90.f);
			
			// 右手指向目标点的最终 Rotator，暴露给动画蓝图使用
			FRotator FindLookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), BongCharacter->GetHitTarget());
			FindLookAtRotation += FRotator(0.f, 0.f, 90.f); // 为了弥补右手插槽的X, Y, Z轴不完全正确的问题
			
			RightHandRotation = FMath::RInterpTo(RightHandRotation, FindLookAtRotation , DeltaSeconds, 30.f); // 修改枪转向突兀的问题
		}

		
		/** Debugging */
		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Socket_MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector X_MuzzleTipTransform(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator() ).GetUnitAxis(EAxis::X) );
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + X_MuzzleTipTransform * 1000.f, FColor::Red);
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BongCharacter->GetHitTarget(), FColor::Green);
	}

	
}













































