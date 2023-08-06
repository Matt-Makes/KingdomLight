// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EMovementDirectionType : uint8
{
	None  UMETA(DisplayName = "None"),
	OrientToMovement  UMETA(DisplayName = "OrientToMovement"),
	Strafe  UMETA(DisplayName = "Strafe")
};
