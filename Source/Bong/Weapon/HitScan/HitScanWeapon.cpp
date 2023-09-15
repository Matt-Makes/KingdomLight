// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Bong/Character/BongCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Bong/BongTypes/MacroHelper.h"
#include "Kismet/KismetMathLibrary.h"


AHitScanWeapon::AHitScanWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 在所有机器执行
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget); // 消耗子弹，开火蒙太奇
	// APawn* PawnInstigator = GetInstigator();
	// if(PawnInstigator == nullptr)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("需要由玩家生成才行，非玩家生成的，比如场景里的武器，那么GetInstigator() 返回的就是空"));
	// }

	
	// ---------- 需要由玩家生成才行，非玩家生成的，比如场景里的武器，那么 GetInstigator()返回的就是空 ---------- //
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* Socket_MuzzleFlash = GetWeaponMesh()->GetSocketByName(FName("Socket_MuzzleFlash"));
	
	//FTransform aa = GetWeaponMesh()->GetSocketByName(FName("Socket_MuzzleFlash"))->GetSocketTransform(GetWeaponMesh()); // 疯狂套娃
	// if(!HasAuthority() && InstigatorController)  UE_LOG(LogTemp, Error, TEXT("在服务端控制，看远程的模拟端对这个判断：大概率不会执行这条Log, 模拟代理，也就是远程代理，在其他机器上的镜像代理，无法本地去控制"));
	// if(HasAuthority() && InstigatorController)  UE_LOG(LogTemp, Error, TEXT("客户端控制，看服务端会不会打印"));
	

	// ---------- 本质上就是看这台机器上 有多少个PlayerController ---------- //
	// ---------- 在服务端，所有 Authority的 InstigatorController都是有效的 ---------- //
	// ---------- 在客户端，只有 Autonomous Proxy的 InstigatorController才有效 ---------- //
	// 这里需要 Simulate Proxy 也能执行
	if(Socket_MuzzleFlash)
	{
		FTransform SocketTransform = Socket_MuzzleFlash->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHitResult; // 数据量比较大
		WeaponTraceHit(Start, HitTarget, FireHitResult);

		if(GetOwner() != FireHitResult.GetActor()) // 避免打到自己
		{
			ABongCharacter* BongCharacter = Cast<ABongCharacter>(FireHitResult.GetActor());
			
			if(BongCharacter && InstigatorController && HasAuthority())
			{
				// 但是只在 服务端施加伤害
				UGameplayStatics::ApplyDamage(
					BongCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
					);
			}
		}
		if(ImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				this,
				ImpactParticle,
				FireHitResult.ImpactPoint,
				FireHitResult.ImpactNormal.Rotation() /* 服务于粒子的方向性 */
				);
		}
		if(HitSound)
		{
			UGameplayStatics::SpawnSoundAtLocation(
				this,
				HitSound,
				FireHitResult.ImpactPoint
				);
		}

	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal(); // 方向确定，单位为1
	
	// HitTarget只起到给定方向的作用 (HitTarget会击中角色，但是不会造成伤害，从枪口到角色身上的点为向量)
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandomVec = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandomVec;
	FVector ToEndLoc = (EndLoc - TraceStart).GetSafeNormal(); // 方向确定，单位为1
	
	// // Debugging  红色为 编辑器内 给定距离绘制球体所达到的位置
	// DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	// DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH), FColor::Cyan, true);

	// 返回一颗弹丸的散射 的终点位置(射程非常远)
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH);
	//return ToEnd * BIG_NUMBER / 100;
}

// 作用1 引用输出Hit命中参数 ; 作用2 仅仅烟雾轨迹Cosmetic
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHitResult)
{
	UWorld* World = GetWorld();
	if(World)
	{
		// 两个的 实际射程都是比实际 HitTarget要远的，HitTarget只是提供一个方向
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;

		// 引用输出参数
		World->LineTraceSingleByChannel(
			OutHitResult,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility);

		// 仅仅烟雾轨迹Cosmetic
		FVector BeamEnd = End;
		if(OutHitResult.bBlockingHit)
		{
			BeamEnd = OutHitResult.ImpactPoint;
		}
		if(BeamParticle)
		{
			UParticleSystemComponent* BeamComp = UGameplayStatics::SpawnEmitterAtLocation(
				this,
				BeamParticle,
				TraceStart,
				FRotator::ZeroRotator,
				true
				);
			if(BeamComp)
			{
				BeamComp->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
		
	}
}
