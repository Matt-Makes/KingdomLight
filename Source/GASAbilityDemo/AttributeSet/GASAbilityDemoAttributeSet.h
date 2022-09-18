// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GASAbilityDemoAttributeSet.generated.h"

/**
 * Ability Demo Attribute Set
 * Contains minimal attributes necessary to demonstrate some basic melee mechanics: Health, MaxHealth, Strength, Endurance, WeaponDamage.
 */
UCLASS()
class UGASAbilityDemoAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	/** Character Health Attribute */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FGameplayAttributeData Health;
	/* Getters and setters for Health. The VALUE_GETTER macro creates a basic const function for getting the current value, the  */
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Health);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(Health);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(Health);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UGASAbilityDemoAttributeSet, Health);

	/** Max Health. Used as a workaround for the Min/Max values not being available in the base implementation of the attribute table */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FGameplayAttributeData MaxHealth;
	GAMEPLAYATTRIBUTE_VALUE_GETTER(MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxHealth);
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UGASAbilityDemoAttributeSet, MaxHealth);

	// 击退效果，可能接管移动组件
	/** Knockback Force. Added as part of the demonstration during the stream. Not implemented, but here for reference. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FGameplayAttributeData KnockbackForceTaken;
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UGASAbilityDemoAttributeSet, KnockbackForceTaken);

	/** Character Strength, used to calculate melee attacks */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FGameplayAttributeData Strength;
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Strength);

	/** Character Endurance, used to calculate defense against attacks */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FGameplayAttributeData Endurance;
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Endurance);

	/** Weapon damage, added on top of the Strength / Endurance calculation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	FGameplayAttributeData WeaponDamage;
	GAMEPLAYATTRIBUTE_VALUE_GETTER(WeaponDamage);
	

}; 
