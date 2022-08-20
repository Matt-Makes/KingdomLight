// Fill out your copyright notice in the Description page of Project Settings.


#include "GECMeleeDamageExecution.h"
#include "GASAbilityDemo/AttributeSet/GASAbilityDemoAttributeSet.h"
#include "AbilitySystemComponent.h"

//Helper struct for fetching the stats necessary for the melee calculation.
struct FMeleeDamageStatics
{
	//Capturedef declarations for attributes.
	DECLARE_ATTRIBUTE_CAPTUREDEF(Strength);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Endurance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(WeaponDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	
	//Default constructor.
	FMeleeDamageStatics()
	{
		//Capturedef definitions for attributes. 

		//Strength and weapon damage from the Source of the Gameplay Effect running this calculation. Snapshotted at the moment the calculation is made.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAbilityDemoAttributeSet, Strength, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAbilityDemoAttributeSet, WeaponDamage, Source, true);

		//Endurance and health from the target of the Gameplay Effect using this calculation.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAbilityDemoAttributeSet, Endurance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UGASAbilityDemoAttributeSet, Health, Target, false);
	}
	
};

//Static helper function to quickly fetch the melee damage capture attributes.
static const FMeleeDamageStatics& MeleeDamageStatics()
{
	static FMeleeDamageStatics DmgStatics;
	return DmgStatics;
}

//Constructor for the Melee Damage Execution. We capture only the attributes used to make the calculation, not the health attribute itself. We apply that at the end of the execution.
UGECMeleeDamageExecution::UGECMeleeDamageExecution()
{
	RelevantAttributesToCapture.Add(MeleeDamageStatics().EnduranceDef);
	RelevantAttributesToCapture.Add(MeleeDamageStatics().StrengthDef);
	RelevantAttributesToCapture.Add(MeleeDamageStatics().WeaponDamageDef);
}

//Main implementation for the execution.
void UGECMeleeDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{

	//Step 1: Obtain references to the target and source Ability System Component. Each copy of a Gameplay Effect will have one of each.
	UAbilitySystemComponent* TargetABSC = ExecutionParams.GetTargetAbilitySystemComponent();
	AActor* TargetActor = TargetABSC ? TargetABSC->GetAvatarActor() : nullptr;

	UAbilitySystemComponent* SourceABSC = ExecutionParams.GetSourceAbilitySystemComponent();
	AActor* SourceActor = SourceABSC ? SourceABSC->GetAvatarActor() : nullptr;

	//Get the owning GameplayEffect Spec so that you can use its variables and tags.
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	//Aggregator Evaluate Parameters used during the attribute capture.
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;


	//Capturing the weapon damage.
	float BaseDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MeleeDamageStatics().WeaponDamageDef, EvaluationParameters, BaseDamage);

	//Capturing Endurance
	float Endurance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MeleeDamageStatics().EnduranceDef, EvaluationParameters, Endurance);

	//Defense will at minimum be 1.
	if (Endurance <= 1.0f)
	{
		Endurance = 1.0f;
	}

	//Capturing Strength.
	float Strength = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MeleeDamageStatics().StrengthDef, EvaluationParameters, Strength);

	//Performing the actual damage calculation. Fianl damage done = Damage * AttackPower / Endurance
	float DamageDone = BaseDamage + (Strength / Endurance);

	//We floor damage at 1.
	if (DamageDone <= 0.f)
	{
		DamageDone = 1.0f;
	}

	//Final execution output. We can add more than one AddOutputModifier to change multiple parameters at a time based on more complicated calculations. Here we apply -DamageDone to to Health. Health itself is clamped in the AttributeSet.
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(MeleeDamageStatics().HealthProperty, EGameplayModOp::Additive, -DamageDone));
}
