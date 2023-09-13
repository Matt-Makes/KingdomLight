// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 
 */
UCLASS()
class BONG_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileBullet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	// 首先命中回调 只在服务端执行
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:
	
	
};
