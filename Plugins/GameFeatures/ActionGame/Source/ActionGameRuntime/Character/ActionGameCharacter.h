// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ActionGameCharacter.generated.h"



class UAG_AbilitySystemComponentBase;
class UAG_AttributeSetBase;

class UGameplayEffect;
class UGameplayAbility;

class UAG_MotionWarpingComponent;
class UAG_CharacterMovementComponent;
class UInventoryComponent;

UCLASS(config=Game)
class AActionGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:

	AActionGameCharacter(const FObjectInitializer& ObjectInitializer);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	
	virtual void Landed(const FHitResult& Hit) override;

	
	// ------------ Attributes Relevant ------------ //
	
	// 自定义辅助函数.
	bool ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext);

protected:

	// 自定义辅助函数 Attributes, Effects and abilities.
	void InitializeAttributes();
	void ApplyStartupEffects();
	void GiveAbilities();
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributeSet;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> DefaultAbilities;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	UPROPERTY(EditDefaultsOnly)
	UActorComponent* AbilitySystemComponent;
	UPROPERTY(Transient)
	UAG_AttributeSetBase* AttributeSet;
	
	
	// 这两个函数用于初始化Everything
	// 服务端
	virtual void PossessedBy(AController* NewController) override;
	// 客户端
	virtual void OnRep_PlayerState() override;

	
};