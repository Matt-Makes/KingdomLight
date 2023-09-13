// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"


AProjectileBullet::AProjectileBullet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile MoveComp"));
	ProjectileMovementComp->bRotationFollowsVelocity = true; // 考虑到重力衰减
	ProjectileMovementComp->SetIsReplicated(true);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Character是 Instigator， 施加伤害者   也是 PawnInstigator(ProjectileRocket)
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if(OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if(OwnerController)
		{
			// DamageCauser的Ownership 是OwnerController
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}

	// 父类 负责执行销毁
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}




