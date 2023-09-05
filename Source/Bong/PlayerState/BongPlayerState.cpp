// Fill out your copyright notice in the Description page of Project Settings.


#include "BongPlayerState.h"
#include "Bong/Character/BongCharacter.h"
#include "Bong/PlayerController/BongPlayerController.h"
#include "Net/UnrealNetwork.h"


ABongPlayerState::ABongPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABongPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABongPlayerState, Defeats);
}

// 只有服务器有资格管理分数
void ABongPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	
	BongCharacter = BongCharacter == nullptr ? Cast<ABongCharacter>(GetPawn()) : BongCharacter;
	if(BongCharacter)
	{
		BongPlayerController = BongPlayerController == nullptr ? Cast<ABongPlayerController>(BongCharacter->Controller) : BongPlayerController;
		if(BongPlayerController)
		{
			BongPlayerController->SetHUDScore(GetScore());
			//UE_LOG(LogTemp, Error, TEXT("PS:  %f"), GetScore());
		}
	}
}

void ABongPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	
	BongCharacter = BongCharacter == nullptr ? Cast<ABongCharacter>(GetPawn()) : BongCharacter;
	if(BongCharacter)
	{
		BongPlayerController = BongPlayerController == nullptr ? Cast<ABongPlayerController>(BongCharacter->Controller) : BongPlayerController;
		if(BongPlayerController)
		{
			BongPlayerController->SetHUDScore(GetScore());
		}
	}
}



void ABongPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	
	BongCharacter = BongCharacter == nullptr ? Cast<ABongCharacter>(GetPawn()) : BongCharacter;
	if(BongCharacter)
	{
		BongPlayerController = BongPlayerController == nullptr ? Cast<ABongPlayerController>(BongCharacter->Controller) : BongPlayerController;
		if(BongPlayerController)
		{
			BongPlayerController->SetHUDDefeats(Defeats);
		}
	}
}

void ABongPlayerState::OnRep_Defeats()
{
	// 不是虚函数，不用Super
	BongCharacter = BongCharacter == nullptr ? Cast<ABongCharacter>(GetPawn()) : BongCharacter;
	if(BongCharacter)
	{
		BongPlayerController = BongPlayerController == nullptr ? Cast<ABongPlayerController>(BongCharacter->Controller) : BongPlayerController;
		if(BongPlayerController)
		{
			BongPlayerController->SetHUDDefeats(Defeats);
		}
	}
}
