// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "NiagaraComponent.h"
//#include "NiagaraFunctionLibrary.h"
#include "RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AProjectileRocket::AProjectileRocket(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	// purely Cosmetic
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Actor组件
	RocketMovementComp = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComp"));
	RocketMovementComp->bRotationFollowsVelocity = true; // 考虑重力下落
	RocketMovementComp->SetIsReplicated(true); // 保险组件复制
	
	/// 类似Timeline组件，不需要实例化
	// NiagaraSystemComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	// ProjectileLoopComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	/// 弹丸都在服务端生成， 命中回调 父类是在服务端，火箭弹子类是在所有客户端，导致在所有机器上都有
	// 构造函数绑定的话，BoxComp还没实例化
	if( !HasAuthority())
	{
		BoxComp->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}
	
	SpawnTrailSystem();
	
	// 火箭弹在空中飞行的循环音效（刚发射的音效是在动画上，和这个无关）
	if(ProjectileLoop && ProjectileLoopAttenuation)
	{
		ProjectileLoopComp = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false, /* 设置逻辑，一旦 Hit就停止播放 */
			1.f,
			1.f,
			0.f,
			ProjectileLoopAttenuation,
			(USoundConcurrency*)nullptr,
			false /* 销毁跟着此 Actor走 */
			);

		//UE_LOG(LogTemp, Display, TEXT(" PlaySound : %s"), *GetNameSafe(GetOwner()));
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor == GetOwner())
	{
		return;
	}

	ExplodeDamage();

	Start_DestroyTimer();
	
	// 播放爆炸特效和音效, 如果蓝图已配置
	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform() );
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation() );
	}
	
	// 不再生成更多粒子，且 隐藏子弹网格体
	if(ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if(BoxComp)
	{
		BoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if(TrailNiagaraSystemComp && TrailNiagaraSystemComp->GetSystemInstanceController())
	{
		TrailNiagaraSystemComp->GetSystemInstanceController()->Deactivate();
	}
	if(ProjectileLoopComp && ProjectileLoopComp->IsPlaying())
	{
		ProjectileLoopComp->Stop();
		//UE_LOG(LogTemp, Display, TEXT(" StopSound : %s"), *GetNameSafe(GetOwner()));
	}

	
	// Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit); // 只负责销毁和 特效音效
}

// 不调用Super, 就不会有爆炸和音效
void AProjectileRocket::Destroyed()
{
	//Super::Destroyed();
}
