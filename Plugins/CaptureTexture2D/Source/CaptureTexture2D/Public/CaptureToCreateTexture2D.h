// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "CaptureToCreateTexture2D.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateTextureSignature,  UTexture2D*,OutTexture);
/**
* 
*/
UCLASS()
class CAPTURETEXTURE2D_API UCaptureToCreateTexture : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	public:                             
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "ScreenCapture")
	static UCaptureToCreateTexture* CaptureToCreateTexture(UObject* WorldContextObject, bool bCaptureUI = true);

	UPROPERTY(BlueprintAssignable)
	FCreateTextureSignature OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FCreateTextureSignature OnFail;

	void ScreenCaptureCompleted(int32 InWidth, int32 InHeight, const TArray<FColor>& InColor);

	private:
	FDelegateHandle CaptureHandle;
};