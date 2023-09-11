// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BONG_API UOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UOverHeadWidget(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString TextToDisplay);
	
	UFUNCTION(BlueprintCallable)
	void ShowPlayerLocalRole(APawn* InPawn);
	UFUNCTION(BlueprintCallable)
	void ShowPlayerRemoteRole(APawn* InPawn);
	
protected:
	// 合适的时机，将UI从 Viewport移除
	virtual void NativeDestruct() override;
	//virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
};
