// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestManager.h"
#include "Blueprint/UserWidget.h"


// Sets default values
AQuestManager::AQuestManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//UE_LOG(LogTemp,Warning, TEXT("QuestManager Constructor"));
}

void AQuestManager::CompleteQuest_Implementation(FName QuestId, bool CompleteWholeQuest)
{
	int32 QIndex = GetQuestIndex(QuestId);
	FQuestInfo Q = QuestList[QIndex];
	if(CompleteWholeQuest)
	{
		QuestList[QIndex].Progress = Q.ProgressTotal;
	}
	else
	{
		QuestList[QIndex].Progress = FMath::Min(Q.Progress+1, Q.ProgressTotal);
	}
	CompletedQuest.Broadcast(QIndex);
	
}

FQuestInfo AQuestManager::GetQuest(FName Name) const
{
	return QuestList[GetQuestIndex(Name)];
}




// Called when the game starts or when spawned
void AQuestManager::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp,Warning, TEXT("QuestManager Beginplay"));
	
}

// Called every frame
void AQuestManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp,Warning, TEXT("QuestManager Tick"));
}


