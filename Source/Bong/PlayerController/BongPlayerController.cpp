// Fill out your copyright notice in the Description page of Project Settings.


#include "BongPlayerController.h"

#include "Bong/BongTypes/MacroHelper.h"
#include "Bong/BongComponents/CombatActorComponent.h"
#include "Bong/BongTypes/Announcement.h"
#include "Bong/Character/BongCharacter.h"
#include "Bong/GameMode/BongGameMode.h"
#include "Bong/GameState/BongGameState.h"
#include "Bong/HUD/Announcement.h"
#include "Bong/HUD/BongHUD.h"
#include "Bong/HUD/CharacterOverlay.h"
#include "Bong/PlayerState/BongPlayerState.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


DEFINE_LOG_CATEGORY(BongPC);



ABongPlayerController::ABongPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABongPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	//UE_LOG(BongPC, Display, TEXT("ReceivePlayer: %f"), GetWorld()->GetTimeSeconds());
	
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds()); // 尽早设置 DeltaClientServer，5秒设置一次
	}
}

// 都是在服务端完成，客户端不执行
void ABongPlayerController::OnPossess(APawn* InPawn)
{
	//UE_LOG(BongPC, Display, TEXT("OnPossess: %f"), GetWorld()->GetTimeSeconds());
	Super::OnPossess(InPawn);

	// Pawn的生效时间 服务端和客户端之间不一致
	ABongCharacter* BongCharacter = Cast<ABongCharacter>(InPawn);
	
	if(BongCharacter)
	{
		// 用于客户端设置 UI
		// HUDHealth = BongCharacter->GetHealth();
		// HUDMaxHealth = BongCharacter->GetMaxHealth();

		// OnPossess 只在服务端设置
		SetHUDHealth(BongCharacter->GetHealth(), BongCharacter->GetMaxHealth());


		// // 服务端执行，不复制，再怎么也影响不到客户端
		// if(GetLocalRole() > ENetRole::ROLE_SimulatedProxy && BongCharacter->IsLocallyControlled())
		// {
		// 	
		// }
		// else // 服务端上的另一个权威性 模拟端
		// {
		// 	
		// }
	}
}

void ABongPlayerController::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(BongPC, Display, TEXT("%f"), GetWorld()->GetTimeSeconds());
	
	BongHUD = Cast<ABongHUD>(GetHUD());
	
	ServerCheckMatchState();
}

void ABongPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	SetHUDTime();
	
	CheckTimeSync(DeltaSeconds); // 5秒设置一次

	PollInit();
	
	if(IsLocalController())
	{
		//UE_LOG(BongPC, Display, TEXT("IsLocalController tick true: %f"), GetWorld()->GetTimeSeconds());
	}
}

void ABongPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABongPlayerController, MatchState);

	// DOREPLIFETIME(ABongPlayerController, bInitializeHealth);
	// DOREPLIFETIME(ABongPlayerController, HUDHealth);
	// DOREPLIFETIME(ABongPlayerController, HUDMaxHealth);
}



#pragma region  M a t c h   S t a t e s



// 这里的PC 思考逻辑时只考虑服务端本地控制和 客户端本地控制，模拟端没有UI
// 真正的倒计时在GameMode， 这里只是模仿，用于显示UI
void ABongPlayerController::ServerCheckMatchState_Implementation()
{
	ABongGameMode* GameMode = Cast<ABongGameMode>(UGameplayStatics::GetGameMode(this));
	// 这里的 LevelStartingTime只会设置一次，但 GM里可以获取更新的
	if(GameMode)
	{
		LevelStartingTime = GameMode->LevelStartingTime;
		CooldownTime = GameMode->CooldownTime;
		MatchTime = GameMode->MatchTime;
		WarmupTime = GameMode->WarmupTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void ABongPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	LevelStartingTime = StartingTime;
	CooldownTime = Cooldown;
	MatchTime = Match;
	WarmupTime = Warmup;

	MatchState = StateOfMatch; // 确保客户端跟着服务端走
	
	// 客户端设置仅影响客户端自己；服务端设置
	OnMatchStateSet(MatchState); // 目前主要是初始化玩家UI，添加到视口和设置值，在同一帧
	
	if(BongHUD && MatchState == MatchState::WaitingToStart)
	{
		BongHUD->AddAnnouncement(); // 服务端的本地控制 调用客户端RPC，也会在本地控制 执行
	}
}


void ABongPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchStateStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

// 只执行在客户端
void ABongPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchStateStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}


// 游戏开始，添加到视口玩家UI，移除提示UI
void ABongPlayerController::HandleMatchStateStarted()
{
	//UE_LOG(BongPC, Display, TEXT("OnMatchStateSet: %f"), GetWorld()->GetTimeSeconds());
	BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(GetHUD()) : BongHUD;
	if(BongHUD)
	{
		if(BongHUD->CharacterOverlay == nullptr) BongHUD->AddCharacterOverlay();
		
		if(BongHUD->Announcement)
		{
			BongHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

// 冷却UI的 添加到视口逻辑，是一种重复利用
void ABongPlayerController::HandleCooldown()
{
	BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(GetHUD()) : BongHUD;
	
	if(BongHUD) // 模拟端的 HUD 永远无法被初始化
	{
		if(BongHUD->CharacterOverlay) BongHUD->CharacterOverlay->RemoveFromParent();
		
		bool bChangeAnnouncement = BongHUD->Announcement &&
			BongHUD->Announcement->AnnouncementText &&
			BongHUD->Announcement->InfoText;
		
		if(bChangeAnnouncement)
		{
			BongHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			BongHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			BongHUD->Announcement->InfoText->SetText(FText());
			
			ABongGameState* BongGameState = Cast<ABongGameState>(UGameplayStatics::GetGameState(this));
			ABongPlayerState* BongPlayerState = GetPlayerState<ABongPlayerState>();
			if(BongGameState && BongPlayerState)
			{
				TArray<ABongPlayerState*> TopPlayerStates = BongGameState->TopScoringPlayers;
				FString InfoTextString = GetInfoText(TopPlayerStates);

				BongHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}

	ABongCharacter* BongCharacter = Cast<ABongCharacter>(GetPawn());
	if(BongCharacter && BongCharacter->GetCombat())
	{
		BongCharacter->bDisableGameplay = true;
		BongCharacter->GetCombat()->FireButtonPressed(false);
	}
}



#pragma endregion 


#pragma region  S y n c i n g   T i m e



void ABongPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float TimeOfServerReceiveClientRequest = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, TimeOfServerReceiveClientRequest); // 服务端调用，在客户端运行，属于第三种情况
}

void ABongPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeOfServerReceiveClientRequest)
{
	float CurrentClientTime = GetWorld()->GetTimeSeconds();
	
	float RoundTripTime = CurrentClientTime - TimeOfClientRequest; // 一个来回
	float CurrentServerTime = TimeOfServerReceiveClientRequest + (RoundTripTime / 2); // 那时 的时间加上已经过去的半个来回，取1/2，就是为了不断的平衡误差
	
	DeltaClientServer = CurrentServerTime - CurrentClientTime; // 服务端的当前时间肯定比客户端的要大
}

void ABongPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

// 从客户端获取当前服务器的时间，所有端口都会调用
float ABongPlayerController::GetServerTime()
{
	if(HasAuthority()) 
	{
		return GetWorld()->GetTimeSeconds();
	}
	
	return GetWorld()->GetTimeSeconds() + DeltaClientServer;
}



#pragma endregion 


// ---------- UI需要统一管理 ---------- //
#pragma region   H U D  &  U I 



// 及时阻断，避免资源竞争，游戏是单线程，不要太耗性能，给Tick内其他函数让出资源，能在上一帧完成，就在上一帧完成
// 要异步加载UI了， 很烦
void ABongPlayerController::PollInit()
{
	// if(ToolCharacterOverlay == nullptr) // 需要有个后期阻断条件
	// {
	// 	// BongHUD早已在BeginPlay初始化
	// 	// BongHUD->CharacterOverlay分别只在本地控制端初始化，通过 ABongPlayerController::OnMatchStateSet ; ABongPlayerController::OnRep_MatchState
	//
	// 	// ABongPlayerController::OnRep_MatchState 和ABongPlayerController::OnPossess在同一帧复制，
	// 	if(BongHUD && BongHUD->CharacterOverlay)
	// 	{
	// 	//if(bInitializeHealth) // 这样 就必须等到复制到客户端才会统一执行，OnPossess虽然速度一致，但没有及时阻断，存在资源竞争
	// 	//{
	// 		ToolCharacterOverlay = BongHUD->CharacterOverlay;
	// 		if(ToolCharacterOverlay)
	// 		{
	// 			//UE_LOG(BongPC, Display, TEXT("HUDHealth111: %f, Time: %f"), HUDHealth, GetWorld()->GetTimeSeconds());
	// 			//if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth); // 避免二次设置，但是如果错过，客户端UI无法初始化
	//
	// 			ABongCharacter* BongCharacter = Cast<ABongCharacter>(GetPawn());
	//
	// 			if(BongCharacter)
	// 			{
	// 				SetHUDHealth(BongCharacter->GetHealth(), BongCharacter->GetMaxHealth());
	// 			}
	// 			
	// 			// 不完全依赖，只在过早的执行失败后，在这里完全合适的时机，再设置一次
	// 			if(bInitializeScore) SetHUDScore(HUDScore);
	// 			if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);
	// 		}
	// 	//}
	// 	}
	// }

	// 重复检查 负责两端的 血条UI初始化 ？
	if (ToolCharacterOverlay == nullptr)
	{
		if (BongHUD && BongHUD->CharacterOverlay)
		{
			ToolCharacterOverlay = BongHUD->CharacterOverlay;
			if(ToolCharacterOverlay)
			{
				// 服务于 服务端
				if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				// 服务于 客户端
				//if(!HasAuthority() && GetLocalRole() > ENetRole::ROLE_SimulatedProxy) // 多此一举，客户端只有一个PC
				if(!HasAuthority())
				{
					ABongCharacter* BongCharacter = Cast<ABongCharacter>(GetPawn());
                    if(BongCharacter)
                    {
                    	SetHUDHealth(BongCharacter->GetHealth(), BongCharacter->GetMaxHealth());
                    }
				}
				
				// 服务于 两端
				if(bInitializeScore) SetHUDScore(HUDScore);
				if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);
			}
		}
	}
}


/// 统筹 计算时间相关的函数  仅仅是更新 UI文本，假的
void ABongPlayerController::SetHUDTime()
{
	// 保险检查
	// if(LevelStartingTime == 0.f)
	// {
	// 	// 加这个全局判断条件的话，编辑器内就无法测试了
	// 	BongGameMode = BongGameMode == nullptr ? Cast<ABongGameMode>(UGameplayStatics::GetGameMode(this)) : BongGameMode;
	// 	if(BongGameMode)
	// 	{
	// 		LevelStartingTime = BongGameMode->LevelStartingTime;
	// 	}
	// }
	
	if(WarmupTime > 0 || MatchTime > 0 || CooldownTime > 0)
	{
		// ---------------------------------------- Tick 重复检查才是王道 -------------------------------------------- //
		if(LevelStartingTime == 0.f)
		{
			// 加这个全局判断条件的话，编辑器内就无法测试了
			BongGameMode = BongGameMode == nullptr ? Cast<ABongGameMode>(UGameplayStatics::GetGameMode(this)) : BongGameMode;
			if(BongGameMode)
			{
				LevelStartingTime = BongGameMode->LevelStartingTime;
			}
		}
		
		float TimeLeft = 0.f;
		if(MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - (GetServerTime() - LevelStartingTime); // GetServerTime() 是自游戏启动以来的全部时间，太大了
		else if(MatchState == MatchState::InProgress) TimeLeft = MatchTime + WarmupTime - (GetServerTime() - LevelStartingTime);
		else if(MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + MatchTime + WarmupTime - (GetServerTime() - LevelStartingTime);
		
		// 可能一帧得上下时间差，WarmupTime=0 且GetServerTime  比LevelStartingTime 大导致出现负数
		int32 SecondsLeft = FMath::CeilToInt(TimeLeft);
		
		if(WarmupTime > 1 && temp0)
		{
			temp0 = false;
			UE_LOG(LogTemp, Warning, TEXT("---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"));
		}
		UE_LOG(LogTemp, Warning, TEXT("WarmupTime: %f, GetServerTime(): %f, LevelStartingTime: %f"), WarmupTime, GetServerTime(), LevelStartingTime);
		
		if(HasAuthority())
		{
			// 目前的机制，最后一名玩家加入大厅后，会立即进入BongMap，所有的逻辑从进入BongMap开始算起
			BongGameMode = BongGameMode == nullptr ? Cast<ABongGameMode>(UGameplayStatics::GetGameMode(this)) : BongGameMode;
			if(BongGameMode) // poll GM
			{
				SecondsLeft = FMath::CeilToInt(BongGameMode->GetCountDownTime());
				//SecondsLeft = FMath::Clamp(FMath::CeilToInt(BongGameMode->GetCountDownTime() + LevelStartingTime), 0, BONG_MAX_NUMBER);
			}
		}
		
		// 可以试着直接传 SecondsLeft
		if(CountdownInt != SecondsLeft) // 后者总是比前者多一秒，直到归0
		{
			if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
			{
				SetHUDAnnouncementCountdown(TimeLeft);
			}
			if(MatchState == MatchState::InProgress)
			{
				SetHUDMatchCountdown(TimeLeft);
			}
		}
		CountdownInt = SecondsLeft;
	}
}

void ABongPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(GetHUD()) : BongHUD;
	
	if(BongHUD && BongHUD->CharacterOverlay && BongHUD->CharacterOverlay->HealthBar && BongHUD->CharacterOverlay->HealthText)
	{
		float PercentHealth = Health / MaxHealth;
		BongHUD->CharacterOverlay->HealthBar->SetPercent(PercentHealth);
		
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BongHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		//UE_LOG(BongPC, Display, TEXT("HUDHealth222: %f, Time: %f"), Health, GetWorld()->GetTimeSeconds());
		
		// 代表设置失败了，交给Tick
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABongPlayerController::SetHUDScore(float Score)
{
	BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(GetHUD()) : BongHUD;

	if(BongHUD && BongHUD->CharacterOverlay && BongHUD->CharacterOverlay->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BongHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		// 代表设置失败了，交给Tick
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABongPlayerController::SetHUDDefeats(int32 Defeats)
{
	BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(GetHUD()) : BongHUD;

	if(BongHUD && BongHUD->CharacterOverlay && BongHUD->CharacterOverlay->DefeatsAmount)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BongHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		// 代表设置失败了，交给Tick
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABongPlayerController::SetHUDWeaponAmmo(int32 WeaponAmmo)
{
	BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(GetHUD()) : BongHUD;

	if(BongHUD && BongHUD->CharacterOverlay && BongHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), WeaponAmmo);
		BongHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABongPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(GetHUD()) : BongHUD;

	if(BongHUD && BongHUD->CharacterOverlay && BongHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), CarriedAmmo);
		BongHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}

void ABongPlayerController::SetHUDMatchCountdown(float MatchTimeLeft)
{
	BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(GetHUD()) : BongHUD;

	if(BongHUD && BongHUD->CharacterOverlay && BongHUD->CharacterOverlay->MatchCountdownText)
	{
		if(MatchTimeLeft < 0.f)
		{
			BongHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		
		int32 Minutes = FMath::FloorToInt(MatchTimeLeft / 60.f);
		int32 Seconds = MatchTimeLeft - Minutes * 60;

		FString MatchTimeText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BongHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(MatchTimeText));
	}
}

void ABongPlayerController::SetHUDAnnouncementCountdown(float AnnouncementTimeLeft)
{
	BongHUD = BongHUD == nullptr ? Cast<ABongHUD>(GetHUD()) : BongHUD;

	if(BongHUD && BongHUD->Announcement && BongHUD->Announcement->WarmupTimeText)
	{
		if(AnnouncementTimeLeft < 0.f)
		{
			//BongHUD->Announcement->WarmupTimeText->SetText(FText::FromString(FString::Printf(TEXT("fallout76: %f"), AnnouncementTimeLeft)));
			BongHUD->Announcement->WarmupTimeText->SetText(FText());
			return;
		}
		
		int32 Minutes = FMath::FloorToInt(AnnouncementTimeLeft / 60.f);
		int32 Seconds = AnnouncementTimeLeft - Minutes * 60;

		FString WarmupText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BongHUD->Announcement->WarmupTimeText->SetText(FText::FromString(WarmupText));
	}
}


// 比赛结束后，获取 计分板信息
FString ABongPlayerController::GetInfoText(const TArray<ABongPlayerState*>& PlayerStates)
{
	ABongPlayerState* BongPlayerState = GetPlayerState<ABongPlayerState>();
	if(BongPlayerState == nullptr) return FString();

	FString InfoTextString;
	if(PlayerStates.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if(PlayerStates.Num() == 1 && PlayerStates[0] == BongPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if(PlayerStates.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *PlayerStates[0]->GetPlayerName());
	}
	else if(PlayerStates.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for(auto TiedPlayerState : PlayerStates)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayerState->GetPlayerName()));
		}
	}
	
	return InfoTextString;
}


#pragma endregion 

















