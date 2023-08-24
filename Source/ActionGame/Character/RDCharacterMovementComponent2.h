// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActionGameRuntime/ActionGameTypes.h"
#include "GameplayTagContainer.h"
#include "RDCharacterMovementComponent2.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class ACTIONGAMERUNTIME_API URDCharacterMovementComponent2 : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:

	bool TryTraversal(UAbilitySystemComponent* ASC);

	virtual void BeginPlay() override;

	virtual bool CanAttemptJump() const override;

	UFUNCTION(BlueprintPure)
	EMovementDirectionType GetMovementDirectionType() const;

	UFUNCTION(BlueprintCallable)
	void SetMovementDirectionType(EMovementDirectionType InMovementDirectionType);

	UFUNCTION()
	void OnEnforcedStrafeTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

protected:

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UGameplayAbility>> TraversalAbilitiesOrdered;

	UPROPERTY(EditAnywhere)
	EMovementDirectionType MovementDirectionType;

	void HandleMovementDirection();
	
};
