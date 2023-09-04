// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BongGameMode.generated.h"

// 往父类命名空间 MatchState中添加新的自定义变量
namespace MatchState
{
	extern BONG_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

DECLARE_LOG_CATEGORY_EXTERN(BongGM, Log, All);

class ABongCharacter;
class ABongPlayerController;
class ABongGameState;

UCLASS()
class BONG_API ABongGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABongGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(ABongCharacter* EliminatedCharacter, ABongPlayerController* VictimController, ABongPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 16.f;
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	// 在 GM内部的 BeginPlay初始化，获取更新
	// 外部得不到更新，在很早期可能为 0
	float LevelStartingTime = 0.f;
	

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountDownTime = 0.f;
	
	UPROPERTY()
	ABongGameState* BongGameState;

public:
	FORCEINLINE float GetCountDownTime() const { return CountDownTime; }
	
};
