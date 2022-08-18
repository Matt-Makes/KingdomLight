// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "SampleAttributeSet.generated.h"

// 定义一个增加各种Getter和Setter方法的宏
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class GASSAMPLE_API USampleAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	USampleAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(USampleAttributeSet, Health);
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(USampleAttributeSet, MaxHealth)
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, Category = "Physical", ReplicatedUsing = OnRep_Physical)
	FGameplayAttributeData Physical;
	ATTRIBUTE_ACCESSORS(USampleAttributeSet, Physical)
	UFUNCTION()
	void OnRep_Physical(const FGameplayAttributeData& OldValue);

	UPROPERTY(BlueprintReadOnly, Category = "Physical", ReplicatedUsing = OnRep_MaxPhysical)
	FGameplayAttributeData MaxPhysical;
	ATTRIBUTE_ACCESSORS(USampleAttributeSet, MaxPhysical)
	UFUNCTION()
	void OnRep_MaxPhysical(const FGameplayAttributeData& OldValue);

public:
	// 属性修改前回调
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue);

	// GE执行后属性回调
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
