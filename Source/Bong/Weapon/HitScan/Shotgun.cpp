// Fill out your copyright notice in the Description page of Project Settings.

#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Bong/Character/BongCharacter.h"
#include "Bong/PlayerController/BongPlayerController.h"
//#include "Bong/BlasterComponents/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"


AShotgun::AShotgun(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AShotgun::Fire(const FVector& HitTarget)
{
	//Super::Fire(HitTarget);
	// 调用了父类的散射函数，但不需要父类的一般扫描武器的功能(击中音效特效，轨迹烟雾特效)
	// 但需要爷爷类的 子弹更新和动画播放
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	ABongCharacter* OwnerCharacter = Cast<ABongCharacter>(OwnerPawn);
	
	const USkeletalMeshSocket* Socket_MuzzleFlash = GetWeaponMesh()->GetSocketByName(FName("Socket_MuzzleFlash"));
	if(Socket_MuzzleFlash)
	{
		FTransform SocketTransform = Socket_MuzzleFlash->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		TMap<ABongCharacter*, uint32> HitMapping;
		
		for(uint32 i = 0; i < NumberOfPellets; ++i)
		{
			FHitResult FireHitResult; // 数据量比较大
			WeaponTraceHit(Start, HitTarget, FireHitResult); 

			ABongCharacter* BongCharacter = Cast<ABongCharacter>(FireHitResult.GetActor());
			
			if(BongCharacter && HasAuthority() && InstigatorController) // 在服务端且这个 被击中的 Character有自己的PlayerController
			{
				// ----------- 计算有多少个弹丸击中同一个 Character ----------- //
				if(HitMapping.Contains(BongCharacter))   ++HitMapping[BongCharacter];
				else   HitMapping.Emplace(BongCharacter, 1);
			}
			/// 需要再次实现未从 HitScanWeapon继承的功能：伤害施加，击中特效音效
			if(ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					this,
					ImpactParticle,
					FireHitResult.ImpactPoint,
					FireHitResult.ImpactNormal.Rotation(),
					true);
			}
			if(HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHitResult.ImpactPoint,
					0.5f,
					FMath::FRandRange(-0.5f, 0.5f));
			}
		}
		
		for(auto HitPair : HitMapping)
		{
			if(HitPair.Key && HasAuthority() && InstigatorController) // 在服务端且这个 被击中的 Character有自己的PlayerController
			{
				if(HitPair.Key != OwnerCharacter) // 防止自己打到自己
				{
					UGameplayStatics::ApplyDamage(
	                    HitPair.Key,
	                    Damage * HitPair.Value,
	                    InstigatorController,
	                    this,
	                    UDamageType::StaticClass());
				}
			}
		}
		
	}
}







