// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"


AProjectileWeapon::AProjectileWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	// 玩家A 的武器动画蒙太奇还是会在 每个端口 播放
	// @see UCombatActorComponent::FireButtonPressed，  AWeapon::Fire
	Super::Fire(HitTarget);

	// 但是玩家A 的生成弹丸确保只在服务端(客户端镜像会及时更新)；不给客户端权限，防止客户端作弊
	if(!HasAuthority()) return;
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner()); // Damage发起者
	
	const USkeletalMeshSocket* Socket_MuzzleFlash = GetWeaponMesh()->GetSocketByName(FName("Socket_MuzzleFlash"));
	if(Socket_MuzzleFlash)
	{
		// From muzzle flash socket to hit location based on TraceUnderCrosshairs
		FTransform SocketTransform = Socket_MuzzleFlash->GetSocketTransform(GetWeaponMesh() );
		
		// HitTarget若没复制，将独立于服务器之外，客户端产生的HitTarget无法给到服务端
		// @see UCombatActorComponent::TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator ToTargetRotation = ToTarget.Rotation();
		
		// 弹丸类已在蓝图配置，且Damage发起者是一个Pawn
		if(ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner(); // Ownership
			SpawnParams.Instigator = InstigatorPawn;

			// 弹丸的生成在服务端，子弹移动复制逻辑交给内置组件，即使客户端生成也可以同步 
			UWorld* World = GetWorld();
			if(World)
			{
				World->SpawnActor<AProjectile>(ProjectileClass,
					SocketTransform.GetLocation(), ToTargetRotation, SpawnParams); // 只需要 ToTargetRotation足矣
			}
		}
	}
}















































