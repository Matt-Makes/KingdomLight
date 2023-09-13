// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Bong/Character/BongCharacter.h"
#include "Bong/Bong.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"


AProjectile::AProjectile(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	// CDO 开启一个碰撞盒
	BoxComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	
	BoxComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	BoxComp->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 如果蓝图已配置粒子
	if(Tracer)
	{
		// 这个粒子组件会附加到BoxComp
		TracerComp = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			BoxComp,
			FName(),/* 附加Skeletal Mesh上的的骨骼之一的Name */
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition);
	}
	// 只在服务端绑定
	// 构造函数绑定的话，BoxComp还没实例化
	if(HasAuthority())
	{
		BoxComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);

		/** 预防射向天空，销毁太慢 */
		//SetLifeSpan(1.f);
	}

	// BoxComp->IgnoreActorWhenMoving(GetInstigatorController(), true);
	// BoxComp->IgnoreActorWhenMoving(GetOwner(), true);
}




void AProjectile::Start_DestroyTimer()
{
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectile::Finished_DestroyTimer, DestroyTime); // 延时销毁
}

void AProjectile::Finished_DestroyTimer()
{
	Destroy(); // Destroyed() 有爆炸和音效，会被执行
}

void AProjectile::SpawnTrailSystem()
{
	if(TrailNiagaraSystem)
	{
		TrailNiagaraSystemComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailNiagaraSystem,
			GetRootComponent(), /* 和此Actor产生依赖的关键 */
			FName(), /* 没有附着点 */
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false /* 阻止自动销毁 */
			);
	}
}

void AProjectile::ExplodeDamage()
{
	APawn* FiringPawnInstigator = GetInstigator(); // 子类调用时，由Character来生成火箭弹，其是 Instigator，施加伤害者
	if(FiringPawnInstigator == nullptr) return;

	/// 施加伤害, Radial damage have attenuation
	// 因为火箭弹子类，在客户端也进行了命中回调绑定
	if(FiringPawnInstigator && HasAuthority())
	{
		AController* FiringCtrlOwnership = FiringPawnInstigator->GetController();
		if(FiringCtrlOwnership)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				10.f,
				GetActorLocation(),
				DamageInnerRadius,
				DamageOuterRadius,
				1.f, /* 指数级增长，1也就是线性的 */
				UDamageType::StaticClass(),
				TArray<AActor*>(), /* 为空，也就是伤害任何足够近的物体，包括角色自己 */
				this, /* 造成伤害的人 */
				FiringCtrlOwnership /* InstigatorController */
				);
		}
	}
}

// 首先命中回调 只在服务端执行
void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 首先命中回调 只在服务端执行，然后
	// ABongCharacter* BongCharacter = Cast<ABongCharacter>(OtherActor);
	// if(BongCharacter)
	// {
	// 	BongCharacter->MulticastHitReact();
	// }

	// 服务端的销毁，会自动传播到所有客户端（达到了多播效果，但是没有浪费带宽）
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


// 这里不一定是在服务端
// 确保会在每一台机器上<包括服务器>的 本地调用
void AProjectile::Destroyed()
{
	Super::Destroyed();

	
	// if(IsPendingKillPending() == true && IsValid(this))
	// {
		// 如果蓝图已配置
        if(ImpactParticles)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform() );
        }
        if(ImpactSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation() );
        }
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Projectile is not IsPendingKillPending !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"));
	// }
}

