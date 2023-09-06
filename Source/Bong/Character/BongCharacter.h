// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Bong/BongTypes/TurningInPlace.h"
#include "Bong/BongTypes/CombatState.h"
#include "Bong/Interfaces/InteractCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
// EnhancedInput
#include "InputActionValue.h"
#include "BongCharacter.generated.h"


class UCombatActorComponent;
class UAnimMontage;
class UInputAction;
class ABongPlayerController;
class ABongGameMode;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class ABongPlayerState;
// EnhancedInput
class UInputMappingContext;
class AActor;

UCLASS()
class BONG_API ABongCharacter : public ACharacter, public IInteractCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABongCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // 共有接口访问私有的输入变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();
	
	void Eliminate();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate();
	
	//UFUNCTION(NetMulticast, Unreliable)
	//void MulticastHitReact();
	void CalculateAO_Pitch();
	virtual void OnRep_ReplicatedMovement() override; // Actor.h 563 Used for replication of our RootComponent's position and velocity
	void SimProxiesTurn(); // 模拟代理的瞄准偏移Yaw动画就是要被放弃的<不重要且耗性能>，Apex和吃鸡的手游，甚至自主代理都没有瞄准偏移的Yaw动画，就一个bUseControllerRotationYaw = true

	virtual void Destroyed() override;

	
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	// 也会被 PC调用
	void UpdateHUDHealth();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
protected:
	virtual void BeginPlay() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float TurnRateGamepad;
	
	// Enhanced Input
	void MoveForward(const FInputActionValue& InputActionValueValue);
	void MoveRight(const FInputActionValue& InputActionValueValue);
	void Turn(const FInputActionValue& InputActionValueValue);
	void LookUp(const FInputActionValue& InputActionValueValue);
	
	virtual void Jump() override;
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	
	void FireButtonPressed();
	void FireButtonReleased();
	void AimButtonPressed();
	void AimButtonReleased();
	// 用于瞄准偏移
	void AimOffset(float Delta);

	// 伤害回调
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// Poll for any relevant classes and initialize our HUD
	void PollInit();
	void OnPlayerStateInitialized();
	void RotatingInPlace(float DeltaTime);
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
	//无法放到构造函数初始化 TSubclassOf<class UWidgetComponent*> OverHeadWidget;

	// 每帧同步不代表每帧复制
	// 同步频率和帧数相同（同一帧：里面修改的属性只有最后的那个值<变化好几次这种情况>会传到客户端）
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	// 此形参代表相关变量接受复制前 缓存当时的值，变量复制完成后此函数执行
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);


	/*
	 * Bong Components
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatActorComponent* Combat;
	
	// Tick函数里别用 可靠RPC (拾取这个操作要有信任感，所以必须可靠
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(); // UE会在Engine Hood创建 actual function definition

	float AO_Yaw; // 插值缓存，用于设置转身枚举
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaSeconds);

	/*
	 * Animation Montage
	 */
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TObjectPtr<UAnimMontage> AM_FireWeapon;
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TObjectPtr<UAnimMontage> AM_Reload;
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TObjectPtr<UAnimMontage> AM_HitReact;
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TObjectPtr<UAnimMontage> AM_Elim;
	
	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	
	float TimeSinceLastMovementReplication;
	UPROPERTY(EditAnywhere)
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	
	float CalculateSpeed();

	/*
	 * Player health
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health();
	UPROPERTY()
	ABongPlayerController* BongPlayerController;

	bool bEliminated = false;
	FTimerHandle ElimTimer;
	
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	void ElimTimerFinished();

	/*
	 * Dissolve material
	 */
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat InterpFunc_DissolveTimelineTrack; // 用于添加关键帧的轨道 委托

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveFloat> FloatCurve_DissolveAsset; // 曲线资产

	// 更新Timeline时接受曲线上对应的浮点Value，利用回调函数用于 调用每一帧，
	UFUNCTION()
	void UpdateDissolveMaterialInstance(float DissolveValueOutput);
	void StartDissolve();

	// Material instance set on blueprint, used with dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	TArray<UMaterialInstance*> DissolveMaterialInstances;
	// Store dynamic material instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	TArray<UMaterialInstanceDynamic*> DynamicDissolveMaterialInstances{0, 0, 0, 0, 0, 0, 0};


	/*
	 * Eliminate bot
	 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> ElimBotVFX;

	// 没有反射，但不用担心垃圾回收
	TObjectPtr<UParticleSystemComponent> ElimBotComp;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> ElimBotSound;

	



	
	UPROPERTY()
	ABongGameMode* BongGameMode;

	UPROPERTY()
	ABongPlayerState* BongPlayerState;
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);

	// 相同套路，动画蓝图需求 调用
	bool IsEquippedWeapon();
	AWeapon* GetEquippedWeapon();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated() const {return bEliminated; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatActorComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetAM_Reload() const { return AM_Reload; }



















	
// Enhanced Input
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Context", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> IMC_Started;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Context", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> IMC_Triggered_MoveBase;

	// Triggered
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_MoveForward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_MoveRight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Turn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_LookUp;

	// Started
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Pickup_EquipWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Crouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Aim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Fire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Reload;
};
