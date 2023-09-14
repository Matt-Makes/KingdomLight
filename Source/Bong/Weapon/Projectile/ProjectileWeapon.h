// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bong/Weapon/Weapon.h"
#include "ProjectileWeapon.generated.h"


class AProjectile;

UCLASS()
class BONG_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AProjectileWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectileClass;
};
