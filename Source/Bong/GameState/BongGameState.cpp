// Fill out your copyright notice in the Description page of Project Settings.


#include "BongGameState.h"

#include "Bong/PlayerState/BongPlayerState.h"
#include "Net/UnrealNetwork.h"


ABongGameState::ABongGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABongGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABongGameState, TopScoringPlayers);
}

void ABongGameState::UpdateTopScore(ABongPlayerState* ScoringPlayer)
{
	if(TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if(ScoringPlayer->GetScore() == TopScore)
	{
		// 自带一层判断，已存在就不添加; 考虑到两个玩家有相同的得分
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if(ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}





