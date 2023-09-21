// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestMarker.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AQuestMarker::AQuestMarker()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>("ParticleSystem");
	SetRootComponent(Root);
	ParticleSystem->SetupAttachment(Root);
}

// Called when the game starts or when spawned
void AQuestMarker::BeginPlay()
{
	//Super::BeginPlay();
	//UE_LOG(LogTemp,Warning, TEXT("QuestManager Beginplay"));

	//(所在对象， &完整引用)
	GetQuestManager()->CompletedQuest.AddDynamic(this, &AQuestMarker::QuestUpdated);
	RefreshVisibility();
}

void AQuestMarker::RefreshVisibility()
{
	FQuestInfo Quest = GetQuestManager()->GetQuest(QuestName);
	bool NewVisibility = GetQuestManager()->IsActiveQuest(/*Quest.QuestID*/QuestName) && Quest.Progress ==
		ShowAtProgress;
	ParticleSystem->SetVisibility(NewVisibility);
}

void AQuestMarker::QuestUpdated(int32 Index)
{
	RefreshVisibility();
}
