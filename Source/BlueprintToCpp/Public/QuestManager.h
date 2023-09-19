// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "FQuestInfo.h"
//#include "VisualizeTexture.h"
#include "GameFramework/Actor.h"

#include "QuestManager.generated.h"




DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCompletedQuestSingnature, int32,Index);

UCLASS()
class BLUEPRINTTOCPP_API AQuestManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AQuestManager();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CompleteQuest(FName QuestId, bool CompleteWholeQuest);

	//QuestMarker的  RefreshVisibility.
	UFUNCTION(BlueprintPure)
	FQuestInfo GetQuest(FName Name) const;
	UFUNCTION(BlueprintPure, BlueprintImplementableEvent)
	bool IsActiveQuest(FName QuestId) const;


	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FCompletedQuestSingnature CompletedQuest;


	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//蓝图可实现事件-获取任务索引。
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintImplementableEvent)
    int32 GetQuestIndex(FName QuestId) const;



public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//任务数组变量。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQuestInfo> QuestList;

	//UPROPERTY(BlueprintAssignable, BlueprintCallable)
	//FCompletedQuestSingnature CompletedQuest;
	

};
