// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AProjectileGrenade::AProjectileGrenade(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // purely Cosmetic

	// 子类构造弹丸运动组件，并开启复制
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile MoveComp"));
	ProjectileMovementComp->bRotationFollowsVelocity = true; // 考虑到重力衰减
	ProjectileMovementComp->SetIsReplicated(true);
	ProjectileMovementComp->bShouldBounce = true;
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage(); // 倒计时，销毁了才施加伤害，而不是触碰到就施加伤害
	
	Super::Destroyed();
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay(); // 跳过 Projectile类

	SpawnTrailSystem();
	Start_DestroyTimer();

	ProjectileMovementComp->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if(BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation());
	}
}
