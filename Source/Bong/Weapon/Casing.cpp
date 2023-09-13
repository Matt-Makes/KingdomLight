// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


ACasing::ACasing(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CasingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// ECC_WorldDynamic是全部阻挡
	//CasingMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	// 确保开启物理后的产生 HitEvent
	CasingMesh->SetNotifyRigidBodyCollision(true);
	MinShellEjectionImpulse = 6.f;
	MaxShellEjectionImpulse = 8.f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);

	// StaticMesh X轴方向，给一个冲力
	CasingMesh->AddImpulse(GetActorForwardVector() * FMath::RandRange(MinShellEjectionImpulse, MaxShellEjectionImpulse));

	SetLifeSpan(FMath::RandRange(1.5f, 2.5f));
}

void ACasing::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	// disable hit event so that the sound won't play multiple times
	CasingMesh->SetNotifyRigidBodyCollision(false);
	
	//GetWorld()->GetTimerManager().SetTimer(ShellDestructionTimer, this, &ACasing::DestroyShell, 2);
}

// void ACasing::DestroyShell()
// {
// 	//GetWorld()->GetTimerManager().ClearTimer(ShellDestructionTimer);
// 	
// 	Destroy();
// }
