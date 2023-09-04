// Fill out your copyright notice in the Description page of Project Settings.


#include "BongGameMode.h"
#include "Bong/Character/BongCharacter.h"
#include "Bong/PlayerController/BongPlayerController.h"
//#include "GameFramework/PlayerState.h"
#include "Bong/GameState/BongGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Bong/PlayerState/BongPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}


DEFINE_LOG_CATEGORY(BongGM);


ABongGameMode::ABongGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Let GM stay in the WaitingToStart state
	bDelayedStart = true;
}

void ABongGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
	UE_LOG(BongGM, Display, TEXT("Beginplay : %f"), GetWorld()->GetTimeSeconds());

	BongGameState = GetGameState<ABongGameState>();
}

void ABongGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	// 服务端对所有 PlayerController迭代
	for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ABongPlayerController* BongPlayer = Cast<ABongPlayerController>(*Iterator);
		if(BongPlayer)
		{
			BongPlayer->OnMatchStateSet(MatchState);
		}
	}
}

// 只有GM改变 MatchState的权利
void ABongGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if(MatchState == MatchState::WaitingToStart)
	{
		// WorldTime要比 GM的时间跑的更快，所以需要弥补
		CountDownTime = WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		
		if(CountDownTime <= 0.f)
		{
			StartMatch(); // 设为InProgress
		}
	}
	else if(MatchState == MatchState::InProgress)
	{
		CountDownTime = MatchTime + WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		if(CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown); // 设为 Cooldown
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + MatchTime + WarmupTime - (GetWorld()->GetTimeSeconds() - LevelStartingTime);
		if(CountDownTime <= 0.f)
		{
			RestartGame(); // 重启游戏也会 ServerTravel
		}

		// 无法重启游戏的折中方法
		// UWorld* World = GetWorld();
		// if(World)
		// {
		// 	bUseSeamlessTravel = true;
		// 	World->ServerTravel(FString("/Game/Maps/BongMap?listen"));
		// }
	}
}

void ABongGameMode::PlayerEliminated(ABongCharacter* EliminatedCharacter, ABongPlayerController* VictimController, ABongPlayerController* AttackerController)
{
	ABongPlayerState* InstigatorPlayerState = AttackerController ? Cast<ABongPlayerState>(AttackerController->PlayerState) : nullptr;
	ABongPlayerState* EliminatedPlayerState = VictimController ? Cast<ABongPlayerState>(VictimController->PlayerState) : nullptr;
	//ABongGameState* BongGameState = GetGameState<ABongGameState>();

	
	// 杀死自己并不会得分
	if(InstigatorPlayerState && InstigatorPlayerState != EliminatedPlayerState)
	{
		//UE_LOG(LogTemp, Error, TEXT("---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"));
		InstigatorPlayerState->AddToScore(1.f);
	}
	
	// No check if EliminatedPlayerState != InstigatorPlayerState because killing yourself counts as a death.// 修复实弹类 火箭弹 杀死自身：失败计数不增加的问题
	if(EliminatedPlayerState)
	{
		EliminatedPlayerState->AddToDefeats(1);
	}
	
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Eliminate();
	}


	if(BongGameState && InstigatorPlayerState && EliminatedPlayerState)
	{
		BongGameState->UpdateTopScore(InstigatorPlayerState);
	}
	
}

void ABongGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	AActor* StartSpot = nullptr;

	// 关于角色的最远重生地点算法
	if(EliminatedCharacter)
	{
		TArray<AActor*> PlayerStarts;
		TArray<AActor*> PlayerCharacters;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		UGameplayStatics::GetAllActorsOfClass(this, ABongCharacter::StaticClass(), PlayerCharacters);
		
		
		if(PlayerStarts.Num() > 0 && PlayerCharacters.Num() > 0)
		{
			float MaxDistance = 0.f;
            for(AActor* PlayerStart : PlayerStarts)
            {
            	for(AActor* PlayerCharacter : PlayerCharacters)
            	{
            		float CurrentDistance = PlayerStart->GetDistanceTo(PlayerCharacter);
            		if(CurrentDistance > MaxDistance)
            		{
            			MaxDistance = CurrentDistance;
            			StartSpot = PlayerStart;
            		}
            	}
            }
		}
		
		// 告知即将销毁，让Controller可以Repossess另一个Pawn< 可能是设置好CDO >
		EliminatedCharacter->Reset();
		// Character 将被销毁，但PC，PS都会依旧存在
		EliminatedCharacter->Destroy();
	}
	
	if(EliminatedController)
	{
		RestartPlayerAtPlayerStart(EliminatedController, StartSpot);
	}
}


























