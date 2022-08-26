// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Test1AttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API UTest1AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UTest1AttributeSet();


	
private:
	// The current health attribute.  The health will be capped by the max health attribute.  Health is hidden from modifiers so only executions can modify it.
	UPROPERTY(BlueprintReadOnly, Category = "Lyra|Health", Meta = (HideFromModifiers, AllowPrivateAccess = true))
	FGameplayAttributeData Health;

	// The current max health attribute.  Max health is an attribute since gameplay effects can modify it.
	UPROPERTY(BlueprintReadOnly, Category = "Lyra|Health", Meta = (AllowPrivateAccess = true))
	float MaxHealth = 100.f;
	
};
