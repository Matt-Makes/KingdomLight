// Fill out your copyright notice in the Description page of Project Settings.


#include "Test1/Test1BpFuncLibrary.h"

#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

void UTest1BpFuncLibrary::AddAttribute(UAbilitySystemComponent* AbilitySystemComponent)
{
	UAttributeSet* NewSet = NewObject<UAttributeSet>();
	
	// 属性 和 组件 关联起来
	AbilitySystemComponent->GetSpawnedAttributes_Mutable().Add(NewSet);
}
