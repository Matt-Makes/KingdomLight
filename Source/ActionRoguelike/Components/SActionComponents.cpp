// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionComponents.h"

#include "ActionRoguelike/ActionRoguelike.h"


// Sets default values for this component's properties
USActionComponents::USActionComponents()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USActionComponents::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USActionComponents::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	LogOnScreen(this, FString::Printf(TEXT("Started: %s"), *CustomActiveTags.ToString()), FColor::Green);

	// USActionComponent* Comp = GetOwningComponent();	
	// Comp->ActiveGameplayTags.AppendTags(GrantsTags);
	
}

