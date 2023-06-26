// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimbCMC.h"

#include "ClimbCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Kismet/KismetMathLibrary.h"

UClimbCMC::UClimbCMC(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

float UClimbCMC::GetMaxAcceleration() const
{
	if (IsClimbing())
	{
		return MaxClimbingAcceleration;
	}
	
	return Super::GetMaxAcceleration();
}

float UClimbCMC::GetMaxBrakingDeceleration() const
{
	return IsClimbing() ? BrakingDecelerationClimbing : Super::GetMaxBrakingDeceleration();
}

float UClimbCMC::GetMaxSpeed() const
{
	return IsClimbing() ? MaxClimbingSpeed : Super::GetMaxSpeed();
}

void UClimbCMC::PostLoad()
{
	Super::PostLoad();
	PlayerCharacterOwner = Cast<AClimbCharacter>(PawnOwner);
}

void UClimbCMC::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
	Super::SetUpdatedComponent(NewUpdatedComponent);
	 // bMovementInProgress, True during movement update, used internally so that attempts to change CharacterOwner and UpdatedComponent are deferred until after an update.
	if (NewUpdatedComponent && !bMovementInProgress)
	{
		PlayerCharacterOwner = Cast<AClimbCharacter>(PawnOwner);
	}
}

void UClimbCMC::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToClimb = ((Flags & FSavedMove_Character::FLAG_Custom_1) != 0);
	bWantsToClimbDash = ((Flags & FSavedMove_Character::FLAG_Custom_2) != 0);
}

FNetworkPredictionData_Client* UClimbCMC::GetPredictionData_Client() const
{
	checkSlow(CharacterOwner != NULL);
	checkSlow(
		CharacterOwner->GetLocalRole() < ROLE_Authority || (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy &&
			GetNetMode() == NM_ListenServer));
	checkSlow(GetNetMode() == NM_Client || GetNetMode() == NM_ListenServer);

	if (!ClientPredictionData)
	{
		UClimbCMC* MutableThis = const_cast<UClimbCMC*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_Climb(*this);
	}

	return ClientPredictionData;
}

void UClimbCMC::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UClimbCMC::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = GetCharacterOwner()->GetMesh()->GetAnimInstance();
	
	ClimbQueryParams.AddIgnoredActor(GetOwner());

	if (ClimbDashCurve)
	{
		float MinTime;
		ClimbDashCurve->GetTimeRange(MinTime, MaxClimbDashCurveTime);
	}
}

void UClimbCMC::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	if (bWantsToClimb && CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		SweepAndStoreWallHits();
		ComputeSurfaceInfo();

		if (!IsClimbing() && CanStartClimbing())
		{
			OnStartClimbing();
		}
		else if (ShouldStopClimbing() || ClimbDownToFloor())
		{
			OnStopClimbing(DeltaSeconds, 0);
		}
	}
}

void UClimbCMC::PhysCustom(float deltaTime, int32 Iterations)
{
	if (IsClimbing())
	{
		PhysClimbing(deltaTime, Iterations);
	}
	
	Super::PhysCustom(deltaTime, Iterations);
}

void UClimbCMC::PhysClimbing(float deltaTime, int32 Iterations)
{
	// Simulated proxy only care about the climbing state,  depend on the net replication from server to client.
	if (deltaTime < MIN_TICK_TIME || PlayerCharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}
	
	UpdateClimbDashState(deltaTime);

	//为了方便单独抽出一个方法来计算速度.
	ComputeClimbingVelocity(deltaTime);

	Iterations++;
	const FVector OldLocation = UpdatedComponent->GetComponentLocation();

	// move along the climbing surface.
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	
	//Moves component by given Location Change and rotates by given rotation. 
	SafeMoveUpdatedComponent(Adjusted, GetClimbingRotation(deltaTime), true, Hit);
	if (Hit.Time < 1.f)
	{
		// //Climb不需要SteppedUp.
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}
	
	// Velocity based on distance traveled.
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}

	//判断是否爬到了上边缘.
	if (TryClimbUpLedge())
	{
		OnStopClimbing(deltaTime, Iterations);
		return;
	}

	//调整位置贴合墙面.
	SnapToClimbingSurface(deltaTime);

	
	/////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////下面是CMOVE_Flying的实现.


	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	//计算速度
	if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		if( bCheatFlying && Acceleration.IsZero() )
		{
			Velocity = FVector::ZeroVector;
		}
		const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction;
		CalcVelocity(deltaTime, Friction, true, GetMaxBrakingDeceleration());
	}

	ApplyRootMotionToVelocity(deltaTime);

	Iterations++;
	bJustTeleported = false;

	FVector TheOldLocation = UpdatedComponent->GetComponentLocation();
        
	//根据速度计算出此帧需要的位移
	const FVector FrameAdjusted = Velocity * deltaTime;
	FHitResult HitResult(1.f);
	
	//根据计算出的位移更新Location和Rotation
	SafeMoveUpdatedComponent(FrameAdjusted, UpdatedComponent->GetComponentQuat(), true, HitResult);

	if (HitResult.Time < 1.f)
	{
		const FVector GravDir = FVector(0.f, 0.f, -1.f);
		const FVector VelDir = Velocity.GetSafeNormal();
		const float UpDown = GravDir | VelDir;

		bool bSteppedUp = false;
		if ((FMath::Abs(HitResult.ImpactNormal.Z) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(HitResult))
		{
			float stepZ = UpdatedComponent->GetComponentLocation().Z;
			bSteppedUp = StepUp(GravDir, FrameAdjusted * (1.f - HitResult.Time), HitResult);
			if (bSteppedUp)
			{
				TheOldLocation.Z = UpdatedComponent->GetComponentLocation().Z + (TheOldLocation.Z - stepZ);
			}
		}

		if (!bSteppedUp)
		{
			//adjust and try again
			HandleImpact(HitResult, deltaTime, FrameAdjusted);
			SlideAlongSurface(FrameAdjusted, (1.f - HitResult.Time), HitResult.Normal, HitResult, true);
		}
	}

	//根据移动的距离重新计算Velocity
	if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - TheOldLocation) / deltaTime;
	}
	
}

void UClimbCMC::SweepAndStoreWallHits()
{
	const float Radius = PlayerCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float HalfHeight = PlayerCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FVector StartOffset = UpdatedComponent->GetForwardVector() * 20;

	// Avoid using the same Start/End location for a Sweep, as it doesn't trigger hits on Landscapes.
	const FVector Start = UpdatedComponent->GetComponentLocation() + StartOffset;
	const FVector End = Start + UpdatedComponent->GetForwardVector();
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);

	TArray<FHitResult> Hits;
	TArray<AActor*> IgnoreActors;
	const bool HitWall = UKismetSystemLibrary::CapsuleTraceMulti(PlayerCharacterOwner, Start, End, Radius,
		HalfHeight, TraceChannel, false, IgnoreActors,
		DrawDebugTrace, Hits, true, FLinearColor::Yellow);

	HitWall ? CurrentWallHits = Hits : CurrentWallHits.Reset();
}

void UClimbCMC::ComputeSurfaceInfo()
{
	CurrentClimbingNormal = FVector::ZeroVector;
	CurrentClimbingPosition = FVector::ZeroVector;

	if (CurrentWallHits.IsEmpty())
	{
		return;
	}

	const FVector Start = UpdatedComponent->GetComponentLocation();
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);

	// calculate the average of all hit points as the climb position and normal.
	for (const FHitResult& WallHit : CurrentWallHits)
	{
		const FVector End = Start + (WallHit.ImpactPoint - Start).GetSafeNormal() * 120;

		FHitResult AssistHit;
		TArray<AActor*> IgnoreActors;
		UKismetSystemLibrary::SphereTraceSingle(PlayerCharacterOwner, Start, End, 6, TraceChannel, false, IgnoreActors,
		                                        DrawDebugTrace, AssistHit, true, FLinearColor::Gray);

		CurrentClimbingPosition += AssistHit.Location;
		CurrentClimbingNormal += AssistHit.Normal;
	}

	CurrentClimbingPosition /= CurrentWallHits.Num();
	CurrentClimbingNormal = CurrentClimbingNormal.GetSafeNormal();
}

bool UClimbCMC::ShouldStopClimbing() const
{
	const bool bIsOnCeiling = FVector::Parallel(CurrentClimbingNormal, FVector::UpVector);

	// ceiling cannot climb 判断Climb墙面法线是否与天花板平行，默认攀爬不能爬天花板.
	return !bWantsToClimb || CurrentClimbingNormal.IsZero() || bIsOnCeiling;
}

bool UClimbCMC::CanStartClimbing() const
{
	const bool bIsOnCeiling = FVector::Parallel(CurrentClimbingNormal, FVector::UpVector);

	// ceiling cannot climb.
	return bWantsToClimb && !CurrentClimbingNormal.IsZero() && !bIsOnCeiling;
}

bool UClimbCMC::ClimbDownToFloor() const
{
	FHitResult FloorHit;
	if (!CheckFloor(FloorHit))
	{
		return false;
	}

	const bool bOnWalkableFloor = FloorHit.Normal.Z > GetWalkableFloorZ();
	
	const float DownSpeed = FVector::DotProduct(Velocity, -FloorHit.Normal);
	const bool bIsMovingTowardsFloor = DownSpeed >= MaxClimbingSpeed / 3;

	// already climb to the walk floor.
	const bool bIsClimbingFloor = CurrentClimbingNormal.Z > GetWalkableFloorZ();
	
	return (bIsMovingTowardsFloor && bOnWalkableFloor) || (bIsClimbingFloor && bOnWalkableFloor);
}

bool UClimbCMC::CheckFloor(FHitResult& FloorHit) const
{
	const FVector Start = UpdatedComponent->GetComponentLocation() + (UpdatedComponent->GetUpVector() * - 20);
	const FVector End = Start + FVector::DownVector * FloorCheckDistance;

	// todo The floor trace channel set to ECC_WorldStatic temporarily.
	TArray<AActor*> IgnoreActors;
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);
	return UKismetSystemLibrary::LineTraceSingle(PlayerCharacterOwner, Start, End, TraceChannel, false, IgnoreActors,
	                                             DrawDebugTrace,FloorHit, true, FLinearColor::Red);
}

void UClimbCMC::UpdateClimbDashState(float deltaTime)
{
	if (BeginStartClimbDash())
	{
		OnStartClimbDashing();
	}

	if (BeginStopClimbDash())
	{
		OnStopClimbDashing();
	}

	if (bWantsToClimbDash && PlayerCharacterOwner->bIsClimbDashing)
	{
		CurrentClimbDashTime += deltaTime;

		// Dash动画执行时间超过 曲线资产里的时间，就结束.
		if (CurrentClimbDashTime >= MaxClimbDashCurveTime)
		{
			bWantsToClimbDash = false;
			OnStopClimbDashing();
		}
	}

	LastFrameClimbDashState = bWantsToClimbDash;
}

void UClimbCMC::ComputeClimbingVelocity(float deltaTime)
{
	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		if (bWantsToClimbDash && PlayerCharacterOwner->bIsClimbDashing)
		{
			AlignClimbDashDirection();
			const float CurrentCurveSpeed = ClimbDashCurve->GetFloatValue(CurrentClimbDashTime);
			Velocity = ClimbDashDirection * CurrentCurveSpeed;
		}
		else
		{
			// todo get friction from the climbing wall.
			constexpr float Friction = 0.0f;
			constexpr bool bFluid = false;
			CalcVelocity(deltaTime, Friction, bFluid, GetMaxBrakingDeceleration());
		}
	}

	ApplyRootMotionToVelocity(deltaTime);
}

void UClimbCMC::AlignClimbDashDirection()
{
	ClimbDashDirection = FVector::VectorPlaneProject(ClimbDashDirection, CurrentClimbingNormal);
}

FQuat UClimbCMC::GetClimbingRotation(float deltaTime) const
{
	const FQuat Current = UpdatedComponent->GetComponentQuat();

	if (HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity())
	{
		return Current;
	}

	const FQuat Target = FRotationMatrix::MakeFromX(-CurrentClimbingNormal).ToQuat();
	const float RotationSpeed = ClimbingRotationSpeed * FMath::Max(1, Velocity.Length() / MaxClimbingSpeed);

	return FMath::QInterpTo(Current, Target, deltaTime, RotationSpeed);
}

bool UClimbCMC::TryClimbUpLedge() const
{
	if (AnimInstance && AnimInstance->Montage_IsPlaying(ClimbUpLedgeMontage))
	{
		return false;
	}
	
	const float UpSpeed = FVector::DotProduct(Velocity, UpdatedComponent->GetUpVector());
	const bool bIsMovingUp = UpSpeed >= MaxClimbingSpeed / 3;
	
	if (bIsMovingUp && HasReachedEdge())
	{
		if (MoveToClimbUpEdge())
		{
			SetRotationToStand();
		
			AnimInstance->Montage_Play(ClimbUpLedgeMontage);
		}

		// if cannot move to the top floor, just return true to stop climbing.
		return true;
	}
	return false;
}

bool UClimbCMC::HasReachedEdge() const
{
	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const float TraceDistance = Capsule->GetUnscaledCapsuleRadius() * 2.5f;

	return !EyeHeightTrace(TraceDistance);
}

bool UClimbCMC::EyeHeightTrace(const float TraceDistance) const
{
	FHitResult UpperEdgeHit;

	const float BaseEyeHeight = GetCharacterOwner()->BaseEyeHeight;
	const float EyeHeightOffset = IsClimbing() ? BaseEyeHeight + ClimbingCapsuleShrinkHeight : BaseEyeHeight;

	const FVector Start = UpdatedComponent->GetComponentLocation() + UpdatedComponent->GetUpVector() * EyeHeightOffset;
	const FVector End = Start + (UpdatedComponent->GetForwardVector() * TraceDistance);
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);

	TArray<AActor*> IgnoreActors;
	return UKismetSystemLibrary::LineTraceSingle(PlayerCharacterOwner, Start, End, TraceChannel, false, IgnoreActors,
	                                             DrawDebugTrace, UpperEdgeHit, true, FLinearColor::Blue);
}

bool UClimbCMC::IsLocationWalkable(const FVector& CheckLocation) const
{
	const FVector CheckEnd = CheckLocation + (FVector::DownVector * 250);
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_WorldStatic);

	TArray<AActor*> IgnoreActors;
	FHitResult LedgeHit;
	const bool bHitLedgeGround = UKismetSystemLibrary::LineTraceSingle(PlayerCharacterOwner, CheckLocation, CheckEnd, TraceChannel, false, IgnoreActors,
		DrawDebugTrace, LedgeHit, true, FLinearColor::Blue);

	return bHitLedgeGround && LedgeHit.Normal.Z >= GetWalkableFloorZ();
}

bool UClimbCMC::MoveToClimbUpEdge() const
{
	// check there is enough space to walk on the top floor.
	const float CapsuleRadius = PlayerCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	const float CapsuleHalfHeight = PlayerCharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const FVector Start = PlayerCharacterOwner->GetCapsuleComponent()->GetComponentLocation() + FVector(0, 0, CapsuleHalfHeight * 2 + 70);
	const FVector End = UKismetMathLibrary::GetForwardVector(PlayerCharacterOwner->GetActorRotation()) * (CapsuleRadius + DistanceFromSurface) + Start;
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(CollisionChannel);

	TArray<AActor*> IgnoreActors;
	FHitResult CapsuleHit(1.f);
	UKismetSystemLibrary::CapsuleTraceSingle(PlayerCharacterOwner, Start, End, CapsuleRadius, CapsuleHalfHeight, TraceChannel, false,
	                                         IgnoreActors, DrawDebugTrace, CapsuleHit, true, FLinearColor::Red);
	if (!CapsuleHit.bBlockingHit)
	{
		const FVector LineTraceEnd = CapsuleHit.TraceEnd - FVector(0, 0, 200);
		FHitResult Hit(1.f);
		UKismetSystemLibrary::LineTraceSingle(PlayerCharacterOwner, CapsuleHit.TraceEnd, LineTraceEnd, TraceChannel,
		                                      false, IgnoreActors, DrawDebugTrace, Hit, true, FLinearColor::Green);
		if (Hit.IsValidBlockingHit())
		{
			PlayerCharacterOwner->EndClimbByJumpOver(Hit.ImpactPoint);
			return true;
		}
	}

	return false;
}

void UClimbCMC::SetRotationToStand() const
{
	const FRotator StandRotation = FRotator(0, UpdatedComponent->GetComponentRotation().Yaw, 0);
	UpdatedComponent->SetRelativeRotation(StandRotation);
}

void UClimbCMC::SnapToClimbingSurface(float deltaTime) const
{
	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Location = UpdatedComponent->GetComponentLocation();
	const FQuat Rotation = UpdatedComponent->GetComponentQuat();
	
	const FVector ForwardDifference = (CurrentClimbingPosition - Location).ProjectOnTo(Forward);
	
	const FVector Offset = -CurrentClimbingNormal * (ForwardDifference.Length() - DistanceFromSurface);

	constexpr bool bSweep = true;

	const float SnapSpeed = ClimbingSnapSpeed * ((Velocity.Length() / MaxClimbingSpeed) + 1);
	UpdatedComponent->MoveComponent(Offset * SnapSpeed * deltaTime, Rotation, bSweep);
}

// 返回为真很严格，难以开始攀爬.
bool UClimbCMC::BeginStartClimbDash() const
{
	return bWantsToClimbDash != LastFrameClimbDashState && bWantsToClimbDash;
}

// 返回为假很严格，也就是说很容易停止攀爬.
bool UClimbCMC::BeginStopClimbDash() const
{
	return bWantsToClimbDash != LastFrameClimbDashState && !bWantsToClimbDash;
}

void UClimbCMC::OnStartClimbing()
{
	PlayerCharacterOwner->bIsClimbing = true;
	SetMovementMode(MOVE_Custom, CMOVE_Climbing);
	bOrientRotationToMovement = false;
	PlayerCharacterOwner->OnStartClimb();
}

void UClimbCMC::OnStopClimbing(float deltaTime, int32 Iterations)
{
	bWantsToClimbDash = false;
	OnStopClimbDashing();
	bWantsToClimb = false;
	PlayerCharacterOwner->bIsClimbing = false;
	SetMovementMode(MOVE_Walking, 0);
	bOrientRotationToMovement = true;
	StartNewPhysics(deltaTime, Iterations);
	PlayerCharacterOwner->OnStopClimb();
}

void UClimbCMC::OnStopClimbDashing()
{
	PlayerCharacterOwner->bIsClimbDashing = false;
	CurrentClimbDashTime = 0.f;
	ClimbDashDirection = FVector::ZeroVector;
	PlayerCharacterOwner->OnStopClimbDashing();
}

void UClimbCMC::OnStartClimbDashing()
{
	PlayerCharacterOwner->bIsClimbDashing = true;
	CurrentClimbDashTime = 0.f;
	
	InitializeClimbDashDirection();
	PlayerCharacterOwner->OnStartClimbDashing();
}

void UClimbCMC::InitializeClimbDashDirection()
{
	// if acceleration is zero or low enough, dash direction is up vector.
	ClimbDashDirection = UpdatedComponent->GetUpVector();
	const float AccelerationThreshold = MaxClimbingAcceleration / 10;
	if (Acceleration.Length() > AccelerationThreshold)
	{
		ClimbDashDirection = Acceleration.GetSafeNormal();
	}else if (Velocity.Length() > 0)
	{
		ClimbDashDirection = Velocity.GetSafeNormal();
	}
}

FVector UClimbCMC::GetClimbSurfaceNormal() const
{
	return CurrentClimbingNormal;
}

FVector UClimbCMC::GetClimbDashDirection()
{
	if (PlayerCharacterOwner->bIsClimbDashing && PlayerCharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		InitializeClimbDashDirection();
	}
	return ClimbDashDirection;
}

bool UClimbCMC::IsClimbing() const
{
	return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == ECustomMovementMode::CMOVE_Climbing;
}

uint8 FSavedMove_Character_Climb::GetCompressedFlags() const
{
	uint8 flags = Super::GetCompressedFlags();

	if (bWantsToClimb)
	{
		flags |= FLAG_Custom_1;
	}

	if (bWantsToClimbDash)
	{
		flags |= FLAG_Custom_2;
	}
	return flags;
}

bool FSavedMove_Character_Climb::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
                                                float MaxDelta) const
{
	const TSharedPtr<FSavedMove_Character_Climb>& ClimbNewMove = StaticCastSharedPtr<
		FSavedMove_Character_Climb>(NewMove);
	if (bWantsToClimb != ClimbNewMove->bWantsToClimb)
	{
		return false;
	}

	if (bWantsToClimbDash != ClimbNewMove->bWantsToClimbDash)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_Character_Climb::Clear()
{
	Super::Clear();

	bWantsToClimb = false;
	bWantsToClimbDash = false;
}

void FSavedMove_Character_Climb::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
                                            FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (UClimbCMC* const Movement = Cast<UClimbCMC>(
		C->GetCharacterMovement()))
	{
		bWantsToClimb = Movement->bWantsToClimb;
		bWantsToClimbDash = Movement->bWantsToClimbDash;
	}
}

void FSavedMove_Character_Climb::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);
}

bool FSavedMove_Character_Climb::IsImportantMove(const FSavedMovePtr& LastAckedMove) const
{
	const TSharedPtr<FSavedMove_Character_Climb>& ClimbLastAckedMove = StaticCastSharedPtr<
		FSavedMove_Character_Climb>(LastAckedMove);
	if (bWantsToClimb != ClimbLastAckedMove->bWantsToClimb)
	{
		return true;
	}

	if (bWantsToClimbDash != ClimbLastAckedMove->bWantsToClimbDash)
	{
		return true;
	}

	return Super::IsImportantMove(LastAckedMove);
}

FSavedMovePtr FNetworkPredictionData_Client_Character_Climb::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Character_Climb());
}