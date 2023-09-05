// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BongPlayerState.generated.h"


class ABongCharacter;
class ABongPlayerController;


UCLASS()
class BONG_API ABongPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABongPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
	// 只有服务器有资格管理分数
	void AddToScore(float ScoreAmount);
	
	void AddToDefeats(int32 DefeatsAmount);


	/*
	 * Replication notification
	 */
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

private:
	// 不安全的指针被垃圾回收了，最好所有指针加上UPROPERTY()
	UPROPERTY()
	ABongCharacter* BongCharacter;
	UPROPERTY()
	ABongPlayerController* BongPlayerController;

	// Score是UE 在PS里已内置好
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
};
