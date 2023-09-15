// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"



UCLASS()
class BONG_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	AShotgun(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10; // 作用只是在暴露给编辑期时，Editor内无法输入一个负数
};


