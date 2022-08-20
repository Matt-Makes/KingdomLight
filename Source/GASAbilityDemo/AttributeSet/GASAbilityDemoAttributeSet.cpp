// Fill out your copyright notice in the Description page of Project Settings.


#include "GASAbilityDemoAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

//The PostGameplayEffectExecute function handles any logic that should happen after a GameplayEffect is applied to the Ability System Component that owns this AttributeSet. In effect it "listens" for changes to happen.
void UGASAbilityDemoAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	//Clamping Health based on Max Health.
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	//example of using the Knockback Attribute.
	else if (Data.EvaluatedData.Attribute == GetKnockbackForceTakenAttribute())
	{
		//Apply code to the parent using the knockback value.
	}
}