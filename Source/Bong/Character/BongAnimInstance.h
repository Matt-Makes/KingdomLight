// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Bong/BongTypes/TurningInPlace.h"
//#include "Bong/BongTypes/TurningInPlace.h"
#include "BongAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BONG_API UBongAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UBongAnimInstance(const FObjectInitializer& ObjectInitializer);
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = true))
	class ABongCharacter* BongCharacter;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float Speed;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsInAir;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsAccelerating;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bEquippedWeapon;
	
	class AWeapon* EquippedWeapon; // 动画蓝图从Character里拿，前置声明一般在构造函数里初始化，这里是Tick函数

	
	// 关键就是看源头变量是不是已复制，方法两端都会执行，但是源头变量的值需要正确，确保复制就可用于多人，UE已内置好
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bCrouched;
	// 源头变量为战斗组件的自定义复制变量
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bAiming;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float YawOffset;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float AO_Pitch;
	// 计算每把枪的 左手插槽 合适位置，之后在蓝图 用于FaBIK; 世界空间转换为骨骼空间，左手相对于右手参考系
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	FTransform LeftHandTransform;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	ETurningInPlace TurningInPlace;
	// 右手指向目标点的最终 世界空间Rotator，让武器指向和瞄准方向一致
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	FRotator RightHandRotation;
	// 不让模拟代理使用，避免为了同步运动，额外的复制矫正开销
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bLocallyControlled;
	// BongCharacter的 Getter获取；只关心 阻止在模拟代理下RotateRootBone，同时在移动时设为false
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bRotateRootBone;
	// GM监管淘汰游戏规则，ABongCharacter里的多播RPC 来设置 bEliminated，
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bEliminated;
	// 换弹蒙太奇需要关闭左手扶着武器IK
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bUseFABRIK;

	
	// 换弹时，禁用瞄准偏移
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bUseAimOffsets;
	// 换弹时，禁用根据 HitTarget来修改右手Rotator
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bModifyRightHand;
};

























































