// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"


URocketMovementComponent::URocketMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 正在影响类外的功能，无需定义
UProjectileMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);


	// 火箭可以继续前进并处理Hit Event
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//Super::HandleImpact(Hit, TimeSlice, MoveDelta);

	/// 火箭弹什么都不做，仅仅当 BoxComp 检测到 Hit事件时，再爆炸

	
}
