// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bong/Weapon/Weapon.h"
#include "HitScanWeapon.generated.h"


class UParticleSystem;

UCLASS()
class BONG_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AHitScanWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Fire(const FVector& HitTarget) override;

protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget,  FHitResult& OutHitResult);

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	// 这里不需要，放 开火动画里就行
	// UParticleSystem* MuzzleFlash;
	// USoundCue* FireSound;
	UPROPERTY(EditDefaultsOnly)
	USoundCue* HitSound;

	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* ImpactParticle;
	
private:
	
	
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* BeamParticle;

	/*
	 * Trace end with scatter
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
};
