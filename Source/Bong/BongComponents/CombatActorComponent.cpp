// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatActorComponent.h"
#include "Bong/Weapon/Weapon.h"
#include "Bong/Character/BongCharacter.h"
#include "Bong/PlayerController/BongPlayerController.h"
//#include "Bong/HUD/BongHUD.h" 头文件已经包含
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "Bong/BongTypes/MacroHelper.h"
#include "Bong/Character/BongAnimInstance.h"


DEFINE_LOG_CATEGORY(LogCombatComp);


UCombatActorComponent::UCombatActorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatActorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCombatActorComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatActorComponent, bCompAiming);
	DOREPLIFETIME(UCombatActorComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatActorComponent, SingleCarriedAmmo, COND_OwnerOnly); // 只有本地控制的Character才有必要在UI上显示弹药量
}

void UCombatActorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if(BongCharacter)
	{
		BongCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		
		if(BongCharacter->GetFollowCamera())
		{
			DefaultFOV = BongCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}

	if(BongCharacter->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
}

void UCombatActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// 确保只对所有端的 本地控制有效
	if(BongCharacter && BongCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		// 节省每台Machine 的性能，不需要为代理付出太多的代价
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}

	
	/** Debugging */
	if(bCompAiming)
	{
		DrawDebugSphere(GetWorld(), HitTarget, 12.f, 12, FColor::Red);
	}
}


/*
 * 瞄准相关<每个端口都会调用> 自定义源头变量不是UE内置的，需要Server_RPC(其服务端客户端都可以，很巧妙)
 */
void UCombatActorComponent::SetAiming(bool bIsAiming)
{
	if(BongCharacter == nullptr || EquippedWeapon == nullptr) return;
	
	// 确保客户端先流畅执行<否则要等RPC 将bIsAiming复制回来
	bCompAiming = bIsAiming;
	
	// 不用管两端，ServerRPC都是在服务器执行，所以只需确保是不是Owner Controller
	// 若是主办人，即服务端Server-owned actor 调用ServerRPC，那就只在服务器运行<我发给我自己
	// 若是本地是客户端，发RPC给自己的RemoteRole = Authority; 服务端上的Client-owned actor内变量发生变化，就会复制给客户端
	ServerSetAiming(bIsAiming);

	// 等待RPC执行完，先瞄准再移动
	if(BongCharacter)
    {
        BongCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
    }
	// 只需在本地完成
	if(BongCharacter->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWY_SniperRifle)
	{
		BongCharacter->ShowSniperScopeWidget(bIsAiming);
	}
}

void UCombatActorComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	// 该变量的第二次设置，也就是确保只在服务端设置
	bCompAiming = bIsAiming;
	if(BongCharacter)
	{
		BongCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

// 模拟代理没有物理输入操作，无法按下按钮，所以永远无法执行这个函数
void UCombatActorComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed; // 会反复被设为真假
	if(bFireButtonPressed)
	{
		Fire();
	}
}


void UCombatActorComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		ServerFire(HitTarget);							// 1
		if(EquippedWeapon)
		{
			CrosshairsShootingFactor += 0.75f;
		}
	}
	Start_FireTimer();
}

bool UCombatActorComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;

	     if(!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWY_Shotgun)
	     {
		     return  true; // 散弹枪的例外
	     }
	
	return (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied);
}

void UCombatActorComponent::Start_FireTimer()
{
	if(EquippedWeapon == nullptr || BongCharacter == nullptr) return;
	
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &UCombatActorComponent::Finished_FireTimer, EquippedWeapon->FireDelay);
}

void UCombatActorComponent::Finished_FireTimer()
{
	if(EquippedWeapon == nullptr) return;
	bCanFire = true;
	if(bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	// 最后一发子弹射完，可以自动装填
	if(EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

// 服务端捡起装备武器，对 SingleCarriedAmmo的赋值，会更新客户端的UI
void UCombatActorComponent::OnRep_SingleCarriedAmmo()
{
	BongController = BongController == nullptr ? Cast<ABongPlayerController>(BongCharacter->Controller) : BongController;
	if(BongController)
	{
		BongController->SetHUDCarriedAmmo(SingleCarriedAmmo);
	}
	// 服务端的逻辑执行后，客户端执行自己的逻辑，需要确保是散弹枪种类
	if(CombatState == ECombatState::ECS_Reloading && EquippedWeapon != nullptr && EquippedWeapon->GetWeaponType() == EWeaponType::EWY_Shotgun && SingleCarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatActorComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMapping.Emplace(EWeaponType::EWY_AssaultRifle, StartingRifleAmmo);
	CarriedAmmoMapping.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMapping.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
	
	CarriedAmmoMapping.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMapping.Emplace(EWeaponType::EWY_SMG, StartingSMGAmmo);
	CarriedAmmoMapping.Emplace(EWeaponType::EWY_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMapping.Emplace(EWeaponType::EWY_SniperRifle, StartingSniperAmmo);
}

// ServerRPC无论如何都在服务器执行
void UCombatActorComponent::ServerFire_Implementation(const FVector_NetQuantize& AimHitTarget)
{
	MulticastFire(AimHitTarget);							// 2
}

// 会在服务器和所有客户端执行，例如玩家A 的武器动画蒙太奇会在 每个端口 播放
void UCombatActorComponent::MulticastFire_Implementation(const FVector_NetQuantize& AimHitTarget)
{
	if(EquippedWeapon == nullptr) return;

	// 散弹枪 例外
	if(BongCharacter && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWY_Shotgun)
	{
		BongCharacter->PlayFireMontage(bCompAiming);
		EquippedWeapon->Fire(AimHitTarget);
		CombatState = ECombatState::ECS_Unoccupied; // 动画蓝图的 AnimNotify的活，现在自己做
		return;
	}
	
    // 按下时再开火
    if(BongCharacter && CombatState == ECombatState::ECS_Unoccupied) // 如果有&& bFireButtonPressed，那么执行失败
    {
        BongCharacter->PlayFireMontage(bCompAiming);
        EquippedWeapon->Fire(AimHitTarget);					// 3
    }
}


/*
 * 只在服务端执行的EquipWeapon，及其相关的一切
 */
void UCombatActorComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (BongCharacter == nullptr || WeaponToEquip == nullptr) return;
	
	// 不能捡起两把武器
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}

	// 核心，求助外部帮手：依赖第一步的 变量复制的先手 后本地Rep_Notify本地先完成，SetWeaponState() 会执行两遍，一遍依赖变量复制，
	// 服务端会再做一遍，可能是矫正，但网络传播需要时间，没有本地的绝对的先后顺序
	EquippedWeapon = WeaponToEquip; // 发生改变，开始复制


	// 服务端执行后，还需要复制到客户端，但这两步 不确定哪一步先到客户端
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_EquippedFirst);
	const USkeletalMeshSocket* HandSocket = BongCharacter->GetMesh()->GetSocketByName(FName("Socket_RightHand"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, BongCharacter->GetMesh() );
	}
	
	// -------------------- 武器的所有权，所有者的UI显示，弹药更新 -------------------- //
	
	EquippedWeapon->SetOwner(BongCharacter); // 武器有了Ownership，那武器上的复制变量就不再需要指定Condition
	EquippedWeapon->SetHUDAmmo(); // 服务端设置武器的Ownership和 服务端的UI更新，客户端UI更新依赖AWeapon::OnRep_Owner
	
	//  在服务端 处理携带不同种武器弹药的 多人复制相关
	if(CarriedAmmoMapping.Contains(EquippedWeapon->GetWeaponType()))
	{
		SingleCarriedAmmo = CarriedAmmoMapping[EquippedWeapon->GetWeaponType()];
	}

	// 服务端更新UI
	BongController = BongController == nullptr ? Cast<ABongPlayerController>(BongCharacter->Controller) : BongController;
	if(BongController)
	{
		BongController->SetHUDCarriedAmmo(SingleCarriedAmmo);
	}
	// 服务端执行
	if(EquippedWeapon->EquippingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquippingSound, BongCharacter->GetActorLocation());
	}
	// 捡起空武器可以自动装填
	if(EquippedWeapon->IsEmpty())
	{
		Reload();
	}
	

	
	/** 装备武器后可以Strafing，注意是在服务端设置的，但是这些变量不好网络复制，所以放在EquippedWeapon该变量的只在客户端执行的Rep_Notify函数里，相当于 手动服务端复制到客户端 */
	BongCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	BongCharacter->bUseControllerRotationYaw = true;
}
/*
 * 即使Owner Machine上的 LocalRole == Autonomous 的 RemoteRole == Authority 已将变量成功设置，但只要没复制，那就影响不到Owner Machine上的Auto（因为他不是来自服务器上的镜像）；
 * 但其他客户端看到的镜像版会被影响，起作用；要影响到Owner Machine上的Auto，就放在只在客户端执行的Rep_Notify里
 * 如果不Rep_Notify会有两个问题：
 * ①一服务端一客户端：服务端成功设置了，客户端没设置；  ②两个客户端：因为中间要走服务器桥接，所以只有Owner Machine不是正常的
 */
void UCombatActorComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && BongCharacter)
	{
		// 不信任网络连接，优先本地设置好
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_EquippedFirst);
		const USkeletalMeshSocket* HandSocket = BongCharacter->GetMesh()->GetSocketByName(FName("Socket_RightHand"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, BongCharacter->GetMesh() );
		}
		// 客户端执行
		if(EquippedWeapon->EquippingSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquippingSound, BongCharacter->GetActorLocation());
		}
		
		BongCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		BongCharacter->bUseControllerRotationYaw = true;
	}
}



// @see TickComponent
void UCombatActorComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport) // simulated proxy 没有视口
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	
	FVector2D CrosshairLocation(ViewportSize.X / 2, ViewportSize.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);
	
	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		// 学习 Lyra, 射击不到 并肩的人，和身后的人，至少要站在并肩靠后半个脚掌的位置才能射中
		// 屏幕发出射线，检测到过近的Character的问题
		// 暂时禁用，取消注释即可恢复
		// if(Character)
		// {
		// 	float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
		// 	Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		// }

		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		/** Debugging */
		//DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red);

		
		// HitTarget的 线性检测，Visibility Channel
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		// 实现了这个接口的Actor，准星就变红
		if(TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}

		
		/** 额外添加，需要注意，因为子弹速度很快，飞出边界可能自动GC */
		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		// 开火按钮按下的一瞬间会绘制一个球体，这里暂时用不上
		//DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		/*if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			//HitTarget = End;
		}
		else // 若命中就绘制球体
		{
			//HitTarget = TraceHitResult.ImpactPoint;
			DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		}*/
	}
}

void UCombatActorComponent::SetHUDCrosshairs(float Delta)
{
	if(BongCharacter == nullptr || BongCharacter->Controller == nullptr) return;
	// 保险措施，预防两帧之间的失效
	BongController = BongController == nullptr ? Cast<ABongPlayerController>(BongCharacter->Controller) : BongController;
	if(BongController)
	{
		BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(BongController->GetHUD()) : BongHUD;
		if(BongHUD)
		{
			// 装备的武器决定了 设置纹理结构体的内容
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
			}
			
			/*
			 * 计算准星扩散和缩小 crosshairs spread
			 */
			FVector2D RangeWalkSpeed(0.f, BongCharacter->GetCharacterMovement()->MaxWalkSpeed); // [0, 600] -> [0, 1]
			FVector2D RangeVelocityMultiplier(0.f, 1.f);

			// 前后左右速度的Value一般 要在RangeWalkSpeed内
			FVector Velocity = BongCharacter->GetVelocity();
			Velocity.Z = 0.f;
			
			CrosshairsVelocityFactor = FMath::GetMappedRangeValueClamped(RangeWalkSpeed, RangeVelocityMultiplier, Velocity.Size());
			if(BongCharacter->GetCharacterMovement()->IsFalling())
			{
				// 想要空中准星扩展更慢，所以用插值<站立不动时，跳起 Velocity.Size() = 0>
				CrosshairsInAirFactor = FMath::FInterpTo(CrosshairsInAirFactor, 2.25f, Delta, 2.25f);
			}
			else
			{
				// 落地 碰到地面时，更快的缩小
				CrosshairsInAirFactor = FMath::FInterpTo(CrosshairsInAirFactor, 0.f, Delta, 30.f);
			}

			if(bCompAiming)
			{
				CrosshairsAimFactor = FMath::FInterpTo(CrosshairsAimFactor, -0.5f, Delta, 30.f);
			}
			else
			{
				CrosshairsAimFactor = FMath::FInterpTo(CrosshairsAimFactor, 0.f, Delta, 30.f);
			}
			
			// 射击时，准星瞬间扩大，之后更快速恢复
			CrosshairsShootingFactor = FMath::FInterpTo(CrosshairsShootingFactor, 0.f, Delta, 40.f);
			
			/** 充分实现移动时跳跃，扩展更大;  站立不动时，跳起 Velocity.Size() = 0 */
			HUDPackage.CrosshairsSpread =
				0.5f/*抵消不动时的瞄准缩小*/ +
				CrosshairsVelocityFactor +
				CrosshairsInAirFactor +
				CrosshairsAimFactor +
				CrosshairsShootingFactor;
			
			
			/*
			 * 给HUD里的 HUDPackage赋值，实时改变
			 */
			BongHUD->SetHUDPackage(HUDPackage);
		}
	}
}


void UCombatActorComponent::Reload()
{
	if(SingleCarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading) // 装备武器时，该变量已赋值
	{
		// 客户端和服务端都会调用，在服务端执行（通知所有客户端我正在换弹）
		ServerReload();
	}
}

/** 只在动画蓝图会调用，只要蒙太奇在播放，就会在结尾 AnimNotify更新弹药计算 */
void UCombatActorComponent::FinishedReloading()
{
	if(BongCharacter == nullptr) return;
	if(BongCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied; // 属性状态 服务端复制到客户端

		// 弹药是复制的，所以只在服务端 做弹药更新
		UpdateAmmoValues(); // 换弹 导致的弹药更新的起点
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatActorComponent::ServerReload_Implementation()
{
	if(BongCharacter == nullptr || EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

// -------------------------- 更新相关 -------------------------- //

void UCombatActorComponent::OnRep_CombatState()
{
	switch(CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed) // 枚举的设置，换弹时不能开火; 实现延迟开火：需要按住开火键不松手
		{
			Fire();
		}
		break;
	}
}

// 只有服务端的蒙太奇的结尾的 AnimNotify会触发此函数
void UCombatActorComponent::UpdateAmmoValues()
{
	if(BongCharacter == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload(); // 会检查，弹夹已填满了，就返回 0

	if(CarriedAmmoMapping.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMapping[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		SingleCarriedAmmo = CarriedAmmoMapping[EquippedWeapon->GetWeaponType()];
	}
	
	BongController = BongController == nullptr ? Cast<ABongPlayerController>(BongCharacter->Controller) : BongController;
	if(BongController)
	{
		BongController->SetHUDCarriedAmmo(SingleCarriedAmmo);
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}

// 只在服务端调用； Shotgun会中断开火，为它单独制作逻辑
void UCombatActorComponent::UpdateShotgunAmmoValues()
{
	if(BongCharacter == nullptr && EquippedWeapon == nullptr) return;
	
	if(CarriedAmmoMapping.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMapping[EquippedWeapon->GetWeaponType()] -= 1; // 一次消耗一发携带的弹药
		SingleCarriedAmmo = CarriedAmmoMapping[EquippedWeapon->GetWeaponType()];
	}

	BongController = BongController == nullptr ? Cast<ABongPlayerController>(BongCharacter->Controller) : BongController;
	if(BongController)
	{
		BongController->SetHUDCarriedAmmo(SingleCarriedAmmo);
	}
	
	EquippedWeapon->AddAmmo(1);
	bCanFire = true; // 可以换弹，中断开火
	// 弹夹已满 或携带的弹药用完了就直接换弹结束
	if(EquippedWeapon->IsFull() || SingleCarriedAmmo == 0)
	{
		// Jump to Shotgun Section
		JumpToShotgunEnd(); // 这里只在服务端执行
	}
}

void UCombatActorComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = BongCharacter->GetMesh()->GetAnimInstance();
	if(AnimInstance || BongCharacter->GetAM_Reload())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

// 被蓝图动画通知调用，蒙太奇的播放是 Multicast
void UCombatActorComponent::ShotgunShellReload()
{
	// 弹药是复制的，所以只在服务端 做弹药更新
	if(BongCharacter && BongCharacter->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}



// -------------------------- 计算相关，两边端口都会执行 -------------------------- //
void UCombatActorComponent::HandleReload()
{
	BongCharacter->PlayReloadMontage();
}

int32 UCombatActorComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr) return 0;
	
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if(CarriedAmmoMapping.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMapping[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}

void UCombatActorComponent::InterpFOV(float Delta)
{
	if(EquippedWeapon == nullptr) return;
	
	if(bCompAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), Delta, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, Delta, ZoomInterpSpeed);
	}
	
	if(BongCharacter && BongCharacter->GetFollowCamera())
	{
		BongCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}











































