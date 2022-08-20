// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GECMeleeDamageExecution.generated.h"

/**
 * Melee Damage Execution Calculation. Used for calculating melee damage based on Strength, Endurance, and Weapon Damage, then applying it to current Health.
 */
UCLASS()
class GASABILITYDEMO_API UGECMeleeDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	public:

	//Constructor. Sets up the attribute captures needed to run the calculation.
	UGECMeleeDamageExecution();

	//Execution implementation. The bulk of this operation is in this function.
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
