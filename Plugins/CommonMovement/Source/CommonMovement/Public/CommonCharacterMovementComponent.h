// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/HitResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathSSE.h"
#include "NativeGameplayTags.h"
#include "UObject/UObjectGlobals.h"

#include "CommonCharacterMovementComponent.generated.h"

class UObject;
struct FFrame;

COMMONMOVEMENT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);

/**
 * FCommonCharacterGroundInfo
 *
 *	Information about the ground under the character.  It only gets updated as needed.
 */
USTRUCT(BlueprintType)
struct FCommonCharacterGroundInfo
{
	GENERATED_BODY()

	FCommonCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};


/**
 * UCommonCharacterMovementComponent
 *
 *	The base character movement component class used by this project.
 */
UCLASS(Config = Game)
class COMMONMOVEMENT_API UCommonCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UCommonCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void SimulateMovement(float DeltaTime) override;

	virtual bool CanAttemptJump() const override;

	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "Common|CharacterMovement")
	const FCommonCharacterGroundInfo& GetGroundInfo();

	void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent interface

protected:

	virtual void InitializeComponent() override;

protected:

	// Cached ground info for the character.  Do not access this directly!  It's only updated when accessed via GetGroundInfo().
	FCommonCharacterGroundInfo CachedGroundInfo;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;
};
