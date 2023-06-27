// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Character/LyraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ClimbCMC.generated.h"

class AClimbCharacter;

/**
 * 
 */
UENUM(BlueprintType)
enum ECustomMovementMode1
{
	CMOVE_Climbing UMETA(DisplayName = "Climbing"),
	CMOVE_MAX UMETA(Hidden),
};

UCLASS()
class GASABILITYDEMO_API UClimbCMC : public UCharacterMovementComponent
{
    GENERATED_BODY()

	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<AClimbCharacter> PlayerCharacterOwner;

public:

    UClimbCMC(const FObjectInitializer& OI);

    virtual float GetMaxAcceleration() const override;

	virtual float GetMaxBrakingDeceleration() const override;

	virtual float GetMaxSpeed() const override;

	virtual void PostLoad() override;

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

public:
	uint8 bWantsToClimb : 1;

	uint8 bWantsToClimbDash : 1;

private:
	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="10.0", ClampMax="900.0"))
	float MaxClimbingSpeed = 120;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="10.0", ClampMax="2000.0"))
	float MaxClimbingAcceleration = 380;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.0", ClampMax="3000.0"))
	float BrakingDecelerationClimbing = 550.f;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.0", ClampMax="60.0"))
	float ClimbingSnapSpeed = 4.f;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="1.0", ClampMax="500.0"))
	float FloorCheckDistance = 90.f;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="1.0", ClampMax="60.0"))
	int ClimbingRotationSpeed = 5;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.0", ClampMax="80.0"))
	float ClimbingCapsuleShrinkHeight = 30;

	UPROPERTY(Category="Character Movement: Climbing", EditAnywhere, meta=(ClampMin="0.0", ClampMax="80.0"))
	float DistanceFromSurface = 45.f;

	UPROPERTY(Category="Character Movement: Climbing", EditDefaultsOnly)
	UAnimMontage* ClimbUpLedgeMontage;

	UPROPERTY(Category="Character Movement: Climbing", EditDefaultsOnly)
	UCurveFloat* ClimbDashCurve;

	UPROPERTY(Category="Character Movement: Climbing", EditDefaultsOnly)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugTrace;

	UPROPERTY()
	UAnimInstance* AnimInstance;

	TArray<FHitResult> CurrentWallHits;

	FCollisionQueryParams ClimbQueryParams;

	FVector CurrentClimbingNormal;

	FVector CurrentClimbingPosition;
	
	// climb dash
	float CurrentClimbDashTime;

	FVector ClimbDashDirection;

	float MaxClimbDashCurveTime;

        // 第一位只有0到1两种可能性.
	uint8 LastFrameClimbDashState : 1;

protected:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual void PhysClimbing(float deltaTime, int32 Iterations);

	void SweepAndStoreWallHits();

	void ComputeSurfaceInfo();

	virtual bool ShouldStopClimbing() const;

	virtual bool CanStartClimbing() const;

	bool ClimbDownToFloor() const;

	bool CheckFloor(FHitResult& FloorHit) const;

	virtual void UpdateClimbDashState(float deltaTime);

	void ComputeClimbingVelocity(float deltaTime);

	/**
	 * Update climb dash direction every tick.
	 */
	virtual void AlignClimbDashDirection();

	virtual FQuat GetClimbingRotation(float deltaTime) const;

	virtual bool TryClimbUpLedge() const;

	/**
	 * Check whether climb to the up edge.
	 */
	virtual bool HasReachedEdge() const;

	bool EyeHeightTrace(const float TraceDistance) const;

	/**
	 * When climbing to the top edge, check whether there are enough spaces to walk on it.
	 */
	bool MoveToClimbUpEdge() const;
	bool IsLocationWalkable(const FVector& CheckLocation) const;

	void SetRotationToStand() const;

	virtual void SnapToClimbingSurface(float deltaTime) const;

	virtual bool BeginStartClimbDash() const;
	
	virtual bool BeginStopClimbDash() const;

	virtual void OnStartClimbing();

	virtual void OnStopClimbing(float deltaTime, int32 Iterations);

	virtual void OnStopClimbDashing();

	virtual void OnStartClimbDashing();

	virtual void InitializeClimbDashDirection();

public:
	
	UFUNCTION(BlueprintPure)
	bool IsClimbing() const;

	FVector GetClimbSurfaceNormal() const;

	FVector GetClimbDashDirection();
};

class GASABILITYDEMO_API FSavedMove_Character_Climb : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;
	
	FSavedMove_Character_Climb()
	{
		
	}

	uint8 bWantsToClimb : 1;

	uint8 bWantsToClimbDash : 1;

	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void Clear() override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* C) override;
	virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;
};

class GASABILITYDEMO_API FNetworkPredictionData_Client_Character_Climb : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_Character_Climb(const UCharacterMovementComponent& ClientMovement)
		: Super(ClientMovement)
	{
	}

	virtual FSavedMovePtr AllocateNewMove() override;
};