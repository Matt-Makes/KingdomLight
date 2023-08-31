// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BongPlayerController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(BongPC, Log, All);

class ABongHUD;
class ABongGameMode;
class ABongPlayerState;
class ABongCharacter;
class UCharacterOverlay;

UCLASS()
class BONG_API ABongPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABongPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/*
	 * 玩家血条
	 */
	void SetHUDHealth(float Health, float MaxHealth);

	/*
	 * 击杀与被杀次数
	 */
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);

	/*
	 * 武器弹药相关
	 */
	void SetHUDWeaponAmmo(int32 WeaponAmmo);
	void SetHUDCarriedAmmo(int32 CarriedAmmo);

	/*
	 * 游戏时间，被 ABongPlayerController::SetHUDTime调用
	 */
	void SetHUDMatchCountdown(float MatchTimeLeft);
	void SetHUDAnnouncementCountdown(float AnnouncementTimeLeft);
	
	/** Synced server world clock */
	virtual float GetServerTime();
	/** Synced with server as soon as possible */
	virtual void ReceivedPlayer() override;

	
	/*
	 * MatchState
	 */

	// @see ABongGameMode::OnMatchStateSet
	void OnMatchStateSet(FName State);
	void HandleMatchStateStarted();
	void HandleCooldown();





	
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();

	/*
	 * Sync time between client and server
	 */
	
	/** For client, requests the current server time, passing the client's time when the request was sent */
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	/** For client, reports the current server time in response to ABongPlayerController::ServerRequestServerTime() */
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeOfServerReceiveClientRequest);
	
	float DeltaClientServer = 0.f; // Difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaSeconds);

	/*
	 * PC需要了解 足够的信息，以满足中途加入游戏
	 */
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();
	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);


	FString GetInfoText(const TArray<ABongPlayerState*>& PlayerStates);
private:
	UPROPERTY()
	ABongHUD* BongHUD;
	UPROPERTY()
	ABongGameMode* BongGameMode;
	
	
	uint32 CountdownInt = 0;
	
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	
	UPROPERTY(ReplicatedUsing = "OnRep_MatchState")
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();
	
	UPROPERTY()
	UCharacterOverlay* ToolCharacterOverlay;

	bool bInitializeHealth, bInitializeScore, bInitializeDefeats;
	float HUDHealth, HUDMaxHealth, HUDScore; // Cached HUD Values used for init
	int32 HUDDefeats;



	

	bool temp0 = true;
};
