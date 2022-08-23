// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MeleeHitboxData.generated.h"

USTRUCT(BlueprintType)
struct FMeleeHitSphereDefinition
{
	GENERATED_BODY()

	public:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName Nickname;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FVector Offset;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float Radius;
};


/**
 * 一个最简单的GAS 近战打击，使用了DA
 * 和 Lyra的近战对比
 */
UCLASS(BlueprintType)
class GASABILITYDEMO_API UMeleeHitboxData : public UDataAsset
{
	GENERATED_BODY()
	
	public:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TArray<FMeleeHitSphereDefinition> MeleeHitSpheres;

	UFUNCTION(BlueprintPure)
	TArray<FMeleeHitSphereDefinition> GetMeleeHitSpheres(TArray<int> indexes);
};
