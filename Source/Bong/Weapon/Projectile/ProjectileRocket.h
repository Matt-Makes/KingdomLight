// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"


class URocketMovementComponent;

UCLASS()
class BONG_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// 重写了，不作为，就可以抵消父类该函数的影响，即覆盖
	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;
	
	// 父类已经 UFUNCTION
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	

	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent* RocketMovementComp;

	
	UPROPERTY()
	UAudioComponent* ProjectileLoopComp;
	UPROPERTY(EditDefaultsOnly)
	USoundCue* ProjectileLoop;
	UPROPERTY(EditDefaultsOnly)
	USoundAttenuation* ProjectileLoopAttenuation;

private:
	
	
	
};
