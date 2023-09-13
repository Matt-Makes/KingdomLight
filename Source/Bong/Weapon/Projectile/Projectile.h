// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"


class UBoxComponent;
class UProjectileMovementComponent; // 封装的Actor组件，处理弹丸移动 复制 的逻辑，只要添加就会自动开始处理
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class UNiagaraSystem;
class UNiagaraComponent;


UCLASS()
class BONG_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick(float DeltaTime) override;
	/** Called when this actor is explicitly being destroyed during gameplay or in the editor, not called during level streaming or gameplay ending */
	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;
	void Start_DestroyTimer();
	void Finished_DestroyTimer();
	// Niagara特效烟雾轨迹
	void SpawnTrailSystem();
	void ExplodeDamage();
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	// 弹丸命中相关
 	UPROPERTY(EditAnywhere)
 	TObjectPtr<UParticleSystem> ImpactParticles;
 	UPROPERTY(EditAnywhere)
 	TObjectPtr<USoundCue> ImpactSound;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxComp;

	// 父类仅仅声明，交给子类选择性实现
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComp;

	UPROPERTY()
	UNiagaraComponent* TrailNiagaraSystemComp; // 存储生成的粒子，对其进行 runtime控制
	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* TrailNiagaraSystem;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> Tracer;
	
	// 一个存储变量，用于之后访问。是个组件而已
	UPROPERTY()
	UParticleSystemComponent* TracerComp;

	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

public:
	
};


























