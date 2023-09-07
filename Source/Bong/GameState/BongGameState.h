// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BongGameState.generated.h"


class ABongPlayerState;

UCLASS()
class BONG_API ABongGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	ABongGameState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(ABongPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ABongPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
	
};
