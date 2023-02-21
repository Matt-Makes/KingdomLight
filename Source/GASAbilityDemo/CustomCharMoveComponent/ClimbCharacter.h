// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ClimbCharacter.generated.h"

UCLASS()
class GASABILITYDEMO_API AClimbCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AClimbCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;




protected:

	/** Called for forwards/backward input */
	UFUNCTION(BlueprintCallable, Category = Controller)
	void MoveForward(float Value);

	/** Called for side to side input */
	UFUNCTION(BlueprintCallable, Category = Controller)
	void MoveRight(float Value);

	UFUNCTION()
	virtual void OnRep_IsClimbing();

	UFUNCTION()
	virtual void OnRep_IsClimbDashing();

	UFUNCTION(BlueprintImplementableEvent, Category="Climb", meta = (DisplayName = "End Climb By Jump Over"))
	void k2_EndClimbByJumpOver();

public:

	/** Set by character movement to specify that this Character is currently climbing. */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = "OnRep_IsClimbing", Category=Climb)
	uint8 bIsClimbing : 1;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = "OnRep_IsClimbDashing", Category=Climb)
	uint8 bIsClimbDashing : 1;

	UFUNCTION(BlueprintCallable, Category=Climb)
	void StartClimbing();

	UFUNCTION(BlueprintCallable, Category=Climb)
	void StopClimbing();

	UFUNCTION(BlueprintCallable, Category=Climb)
	bool IsClimbingMoving();

	UFUNCTION(BlueprintCallable, Category=Climb)
	virtual void ClimbDash();

	virtual void OnStartClimb();

	virtual void OnStopClimb();

	virtual void EndClimbByJumpOver(const FVector& JumpOverLocation);

	virtual void OnStartClimbDashing();

	virtual void OnStopClimbDashing();
};
