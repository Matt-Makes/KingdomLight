// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionGameCharacter.h"

AActionGameCharacter::AActionGameCharacter(const FObjectInitializer& ObjectInitializer)
{
}

UAbilitySystemComponent* AActionGameCharacter::GetAbilitySystemComponent() const
{
}

void AActionGameCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
}

bool AActionGameCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect,
	FGameplayEffectContextHandle InEffectContext)
{
}

void AActionGameCharacter::InitializeAttributes()
{
}

void AActionGameCharacter::ApplyStartupEffects()
{
}

void AActionGameCharacter::GiveAbilities()
{
}

void AActionGameCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AActionGameCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}
