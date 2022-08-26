// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Test1BpFuncLibrary.generated.h"

class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API UTest1BpFuncLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		static void AddAttribute(UAbilitySystemComponent* AbilitySystemComponent);
	
};
