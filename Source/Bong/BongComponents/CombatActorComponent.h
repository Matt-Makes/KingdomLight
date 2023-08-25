// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bong/HUD/BongHUD.h"
#include "Bong/Weapon/WeaponTypes.h"
#include "Bong/BongTypes/CombatState.h"
#include "Components/ActorComponent.h"
#include "CombatActorComponent.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogCombatComp, Log, All);


class ABongPlayerController;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BONG_API UCombatActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatActorComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	friend class ABongCharacter; //友元类
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();
	
	void FireButtonPressed(bool bPressed); // 被PC访问
	
	// 只在蓝图的动画通知使用，C++ 里没有调用(动画蒙太奇被多播调用
	UFUNCTION(BlueprintCallable)
	void FinishedReloading();
	// 只在蓝图的动画通知使用，C++ 里没有调用(动画蒙太奇被多播调用
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	// RPC不像Rep_Notify，是可以有参数的
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	UFUNCTION()
	void OnRep_EquippedWeapon();


	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& AimHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& AimHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUDCrosshairs(float Delta);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	
	void HandleReload();
	int32 AmountToReload();
	
private:
	// Runtime阶段用this指针拿到Character对象，目的：要知道是谁装备的武器
	UPROPERTY()
	ABongCharacter* BongCharacter; //角色自带复制开启
	UPROPERTY()
	ABongPlayerController* BongController;
	UPROPERTY()
	ABongHUD* BongHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(Replicated)
	bool bCompAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/** 
	* HUD and crosshairs
	*/
	float CrosshairsVelocityFactor;
	float CrosshairsInAirFactor;
	float CrosshairsAimFactor;
	float CrosshairsShootingFactor;

	FVector HitTarget;

	FHUDPackage HUDPackage;
	
	/*
	 * Aiming and FOV
	 */

	// Field of view when not aiming; set to the camera's base FOV in Beginplay
	float DefaultFOV;
	float CurrentFOV;

	void InterpFOV(float Delta);
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.f;

	/*
	 * Automatic fire
	 */
	FTimerHandle FireTimer;
	bool bCanFire = true;
	bool CanFire();

	void Start_FireTimer();
	void Finished_FireTimer();

	/*
	 * Ammo
	 */
	
	// Carried Ammo for the Currently-equipped weapon
	// 单一 一次性变量，不放在PS里原因：组件依附于Character，其复制速度比PS快
	UPROPERTY(ReplicatedUsing = OnRep_SingleCarriedAmmo)
	int32 SingleCarriedAmmo;
	
	UFUNCTION()
	void OnRep_SingleCarriedAmmo();
	
	TMap<EWeaponType, int32>  CarriedAmmoMapping;

	UPROPERTY(EditAnywhere)
	int32 StartingRifleAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;
	
	void InitializeCarriedAmmo();


	// 其他类通过战斗组件来获得CombatState
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	
public:	
	// 绑定到 USphere组件的 开始重叠函数
	// 自定义的带反射的虚函数，回调函数










	










	
};