// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/PrimitiveComponent.h"

#include "Grabber.generated.h"



//class UUserWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class BLUEPRINTTOCPP_API UGrabber : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrabber();

	UFUNCTION(BlueprintCallable)
	void Grab();

	UFUNCTION(BlueprintCallable)
	void Release();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxGrabDistance = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float HoldDistance = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float GrabRadius = 50.0f;
	// TSubclassOf<UUserWidget> NewWidgetClass;


	//蓝图可实现事件。
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void NotifyQuestActor(AActor* Actor);

	//蓝图本地事件。
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool TraceForPhysicsBodies(AActor*& HitActor, UPrimitiveComponent*& HitComponent);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	//Vector可以表示点的坐标，也可以表示向量，UE4一般把单位向量表示方向。
	//HoldLocation是以WorldLocation为坐标原点的，看源码。
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetHoldLocation() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetMaxGrabDistance() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UPhysicsHandleComponent* GetPhysicsComponent() const;

		
};
