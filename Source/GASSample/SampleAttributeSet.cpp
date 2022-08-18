// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleAttributeSet.h"
#include "GASSampleCharacter.h"

USampleAttributeSet::USampleAttributeSet():
	Health(100.0f),
	MaxHealth(100.0f),
	Physical(100.0f),
	MaxPhysical(100.0f)
{
	
}

void USampleAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USampleAttributeSet, Health);
	DOREPLIFETIME(USampleAttributeSet, MaxHealth);
	DOREPLIFETIME(USampleAttributeSet, Physical);
	DOREPLIFETIME(USampleAttributeSet, MaxPhysical);
}

void USampleAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USampleAttributeSet, Health, OldValue);
}

void USampleAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USampleAttributeSet, MaxHealth, OldValue);
}

void USampleAttributeSet::OnRep_Physical(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USampleAttributeSet, Physical, OldValue);
}

void USampleAttributeSet::OnRep_MaxPhysical(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USampleAttributeSet, MaxPhysical, OldValue);
}

void USampleAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	if (Attribute == GetPhysicalAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxPhysical());
	}
}

void USampleAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	// 仅在instant GameplayEffect使Attribute的 BaseValue改变时触发。
	Super::PostGameplayEffectExecute(Data);

	AGASSampleCharacter* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetCharacter = Cast<AGASSampleCharacter>(TargetActor);
	}
	if (nullptr != TargetCharacter)
	{
		if (Data.EvaluatedData.Attribute == GetHealthAttribute())
		{
			TargetCharacter->OnHealthChanged();
		}
		if (Data.EvaluatedData.Attribute == GetPhysicalAttribute())
		{
			TargetCharacter->OnPhysicalChanged();
		}
	}
	
}
