// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(Blueprintable)
enum class EWeaponState : uint8
{
	EWS_Initial					UMETA(DisplayName = "Initial State"),
	EWS_EquippedFirst			UMETA(DisplayName = "Equipped First"),
	EWS_EquippedSecond			UMETA(DisplayName = "Equipped Second"),
	EWS_EquippedThird			UMETA(DisplayName = "Equipped Third"),
	EWS_Dropped					UMETA(DisplayName = "Dropped"),
	
	EWS_MAX						UMETA(DisplayName = "DefaultMAX")
};
UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan					UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile				UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun					UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX						UMETA(DisplayName = "DefaultMAX")
};


class UTexture2D;
class USkeletalMeshComponent;
class USphereComponent;
class UAnimationAsset;
class UWidgetComponent;
class ACasing;
class ABongCharacter;
class ABongPlayerController;
class USoundCue;


UCLASS()
class BONG_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override; // Called when owner changes，很适合用于那些Ownership经常改变的东西
	void SetHUDAmmo();
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);
	
	// 十字准线在武器上，取决于武器
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsCenter;
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsTop;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsBottom;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	TObjectPtr<UTexture2D> CrosshairsRight;
	
	/*
	 * Zoomed FOV while aiming
	 */
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/*
	 * Automatic fire
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> EquippingSound;

	/*
	 * Enable or disable custom depth
	 */
	void EnableCustomDepth(bool bEnable);
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
	TObjectPtr<UAnimationAsset> FireAnimation;
	// 不需要在构造函数初始化，Runtime时用于SpawnActor
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

	/*
	 * Ammo
	 */

	// 武器枪膛内出生时 自带的弹药量
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo = 30;
	
	void SpendRound();
	
	UFUNCTION()
	void OnRep_Ammo();
	
	UPROPERTY(EditAnywhere)
	int32 MagCapacity = 30;

	// 涉及到扔枪，捡枪的Ownership
	UPROPERTY()
	ABongCharacter* BongOwnerCharacter;
	UPROPERTY()
	ABongPlayerController* BongOwnerPlayerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	
	
public:	
	void SetWeaponState(EWeaponState State);
	
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	bool IsFull();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};


























