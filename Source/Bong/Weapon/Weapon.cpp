// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Bong/Character/BongCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Bong/PlayerController/BongPlayerController.h"
#include "Bong/BongComponents/CombatActorComponent.h"
//#include "Bong/Weapon/WeaponTypes.h"

AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	// 让该Actor对象里的状态属性可复制，总开关
	bReplicates = true;
	// 确保客户端和服务端的落点位置 完全一样，以免捡武器没问题
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SetRootComponent(WeaponMesh);

	// 关闭碰撞流程（总结就是后面会覆盖前面的，越往后优先级越重要。 服务端客户端先 设置同一个CDO
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); //人物可以走过去
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 解决人物被杀，枪掉落和相机卡位的问题，其实还有碰撞通道ECC_SkeletalMesh
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); //需要时再开启

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty(); // 强制刷新
	EnableCustomDepth(true);
	

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// if(GetLocalRole() == ENetRole::ROLE_Authority)
	if (HasAuthority())
	{
		// 开启碰撞流程
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		// 只希望交叠事件放在服务器，Server统一安排
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false); //一开始让客户端和服务器都为false
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

// 该回调函数只绑定在了服务器
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABongCharacter* BlasterCharacter = Cast<ABongCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this); //让OverlappingWeapon有效
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABongCharacter* BlasterCharacter = Cast<ABongCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}



// ---------- 多人下的武器弹药复制 相关 ---------- //

void AWeapon::SetHUDAmmo()
{
	BongOwnerCharacter = BongOwnerCharacter == nullptr ? Cast<ABongCharacter>(GetOwner()) : BongOwnerCharacter;
	if(BongOwnerCharacter)
	{
		BongOwnerPlayerController = BongOwnerPlayerController == nullptr ? Cast<ABongPlayerController>(BongOwnerCharacter->Controller) : BongOwnerPlayerController;
		if(BongOwnerPlayerController)
		{
			BongOwnerPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

// 射击在服务端发生，服务端更新子弹，然后客户更新子弹;
void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo-1, 0, MagCapacity);
	SetHUDAmmo();
}
void AWeapon::OnRep_Ammo()
{
	BongOwnerCharacter = BongOwnerCharacter == nullptr ? Cast<ABongCharacter>(GetOwner()) : BongOwnerCharacter;
	// 没子弹装载了，Shotgun自驱动蒙太奇终结
	if(BongOwnerCharacter && BongOwnerCharacter->GetCombat() && IsFull())
	{
		BongOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

// 客户端能更新子弹的根本原因，捡起一把武器后，其Owner改变，更新子弹
// @see UCombatActorComponent::EquipWeapon
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	if(Owner == nullptr)
	{
		// 你丢掉武器，那么这把武器就不再属于你，若BongOwnerCharacter依旧有效，那么别人捡起你的武器，会更新你的UI
		BongOwnerCharacter = nullptr;
		BongOwnerPlayerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}






/** WeaponState不需要指定Cond_OwnerOnly，因为其属于EquippedWeapon->CombatComp->Character->服务端的Controller之一 */
// 服务端统一设置武器状态，利用属性状态同步（武器都在服务端，就算客户端装备武器，也是服务端的RemoteRole == Simulated Proxy在装备武器，这也是服务端要维持那么多Controller的原因，这是一种Authority）

// EquipWeapon()在服务端被调用，但是不信任网络连接，优先客户端设置好 @see  UCombatActorComponent::OnRep_EquippedWeapon
void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	
	switch (WeaponState)
	{
		// ---------- 大体上捡起时，关闭碰撞流程 ---------- //
	case EWeaponState::EWS_EquippedFirst:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 服务端关球体碰撞，以免带着武器和其他玩家碰撞有交叠; 装备武器：需要考虑关闭物理模拟优先级，已经在本地优先设置，不用再判断
		
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 局部上捡起SMG时，开启碰撞流程，为了带子有重力
		// if(WeaponType == EWeaponType::EWY_SMG)
		// {
		// 	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		// 	WeaponMesh->SetEnableGravity(true);
		// }
		
		EnableCustomDepth(false);
		break;


		// ---------- 大体上扔下时，开启碰撞流程 ---------- //
	case EWeaponState::EWS_Dropped:
		if(HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 开启非模型的交互碰撞只有 服务端有资格; Dropped武器：现在客户端 服务端都有可能调用这个函数，所以要确保是服务端
		}
		WeaponMesh->SetSimulatePhysics(true); // 放在后面执行，会在空中停顿
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// // Begin 照顾SMG搞特殊，扔枪后，确保SMG 返回至CDO状态
		// WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		// WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); // 人物可以走过去
		// WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		// // End

		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
		WeaponMesh->MarkRenderStateDirty(); // 强制刷新
		EnableCustomDepth(true);
		break;
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

// C++版Rep_Notify只在客户端运行（每帧属性状态同步：服务端的模拟Controller属性一旦发生变化，就一定发送给客户端，让客户端的玩家可以捡起武器）
void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_EquippedFirst:
		ShowPickupWidget(false); //只在客户端运行，自己独立显示的拾取UI，自己关掉

		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 局部上捡起SMG时，开启碰撞流程，为了带子有重力
		// if(WeaponType == EWeaponType::EWY_SMG)
		// {
		// 	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// 	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		// 	WeaponMesh->SetEnableGravity(true);
		// }

		EnableCustomDepth(false);
		break;

	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// // Begin 照顾SMG搞特殊，扔枪后，确保SMG 返回至CDO状态
		// WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		// WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); // 人物可以走过去
		// WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		// // End

		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
		WeaponMesh->MarkRenderStateDirty(); // 强制刷新
		EnableCustomDepth(true);
		break;
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

// 多播RPC 使Fire() 在每台Machine上执行
void AWeapon::Fire(const FVector& HitTarget)
{
	if(FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	
	/** 生成弹壳的起点 */
	// 弹壳在每台Machine Locally生成，可注释掉，如果蓝图已配置弹壳类
	// if(CasingClass)
	// {
	// 	USkeletalMeshSocket const* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("Socket_AmmoEject"));
	// 	if(AmmoEjectSocket)
	// 	{
	// 		FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
	// 		UWorld* World = GetWorld();
	// 		if(World)
	// 		{
	// 			World->SpawnActor<ACasing>(
	// 				CasingClass,
	// 				SocketTransform.GetLocation(),
	// 				SocketTransform.GetRotation().Rotator());
	// 		}
	// 	}
	// }

	SpendRound();
}

// 为了后续的流程考虑，确保只在服务端调用
void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	const FDetachmentTransformRules DetachRule(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRule);

	// 会调用OnRep_Owner
	SetOwner(nullptr);
	
	// 你丢掉武器，那么这把武器就不再属于你，若BongOwnerCharacter依旧有效，那么别人捡起你的武器，会更新你的UI
	BongOwnerCharacter = nullptr;
	BongOwnerPlayerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}




























