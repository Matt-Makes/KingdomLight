// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RDCharacter.generated.h"



// Declaring our own custom delegates for the Jump_Ability class to bind callback to.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRDCMC_OnPressedClimbDelegate, bool, bPrevWantsToClimb);

UCLASS()
class COMMONMOVEMENT_API ARDCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ARDCharacter();

    //~CMC interface.
    // Climb.
    bool bClimbing;
    bool bHoldRDClimbing;
    FRDCMC_OnPressedClimbDelegate RDCMC_PressedClimbDelegate; // Our own custom delegates for the Menu class to bind callbacks to.
    // Mantle.
    bool bPressedRDJump;
    virtual void Jump() override;
    virtual void StopJumping() override;

    // Facing：角色胶囊体朝向的那一面。 LastMovementInput：玩家输入方向，最后时刻的输入，例如面朝前却向左或向右运动.
    // bool SelectDirectionalSlide(FVector Facing, FVector LastMovementInput);
    // bool IsCardinalForward(const FVector& Accel, const FVector& Velocity, const FVector& ForwardVector = FVector::ZeroVector);
    //~End of CMC interface.

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;





    
	
public:
    // Collision query params.
    FCollisionQueryParams GetIgnoreCharacterParams() const;

    // // 动画蓝图也会用到，看能不能用枚举替代.
    // /** Set by character movement to specify that this Character is currently climbing. */
    // UPROPERTY(BlueprintReadOnly, ReplicatedUsing = "OnRep_IsClimbing", Category=Climb)
    // uint8 bIsClimbing : 1;

    protected:
    /** Called for forwards/backward input */
    UFUNCTION(BlueprintCallable, Category = Controller)
    void MoveForward(float Value);

    /** Called for side to side input */
    UFUNCTION(BlueprintCallable, Category = Controller)
    void MoveRight(float Value);
    
};
