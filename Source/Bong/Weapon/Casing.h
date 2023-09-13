// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

class USoundCue;

UCLASS()
class BONG_API ACasing : public AActor
{
	GENERATED_BODY()

public:
	ACasing(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
	float MinShellEjectionImpulse;
	UPROPERTY(EditAnywhere)
	float MaxShellEjectionImpulse;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> ShellSound;

	// void DestroyShell();
	// FTimerHandle ShellDestructionTimer;
};
