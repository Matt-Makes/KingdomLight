// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "UObject/Object.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "CaptureToCreateTexture2D.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateTextureSignature, UTexture2D*, OutTexture);
/**
* 
*/
UCLASS()
class CAPTURETEXTURE2D_API UCaptureToCreateTextureNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

	/*返回一个GetWorld，上下文世界环境。*/
public:                         
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", bCaptureUI = "true"), Category = "ScreenCapture")
	static UCaptureToCreateTextureNode* CaptureToCreateTexture(UObject* WorldContextObject, bool bCaptureUI);

	//两个事件分发器。
	UPROPERTY(BlueprintAssignable)
	FCreateTextureSignature OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FCreateTextureSignature OnFail;

	//该成员函数用于截图绑定（不需要UFUNCTION，蓝图中不调用），需要有三个重要的输入参数，尤其是Color；GS里拦截Texture是交给这个成员函数的。
	void ScreenCaptureCompleted(int32 InWidth, int32 InHeight, const TArray<FColor>& InColor);


private:
	FDelegateHandle CaptureHandle;

	// UPROPERTY()
	// UTexture2D* Tex;
};






DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintAsyncNodePinResult, int32, Result);
/**
 * 
 */
UCLASS()
class CAPTURETEXTURE2D_API UBlueprintAsyncNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
 
	UPROPERTY(BlueprintAssignable)
	FBlueprintAsyncNodePinResult OnSuccess;
 
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", Delay = "0.5"))
	static UBlueprintAsyncNode* AsyncDelay(UObject* WorldContextObject, float Delay);
 
protected:
	void TimeoutCallback();
};






DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCreateTextureWithStringSignature, UTexture2D*, OutTexture, FString, Options);
/**
 * 
 */
UCLASS()
class CAPTURETEXTURE2D_API UCaptureToCreateTextureWithStringNode : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:                         
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", bCaptureUI = "true"), Category = "ScreenCapture")
	static UCaptureToCreateTextureWithStringNode* CaptureToCreateTextureWithString(UObject* WorldContextObject, bool bCaptureUI, FString Options);

	// 构建多播对象.
	UPROPERTY(BlueprintAssignable)
	FCreateTextureWithStringSignature OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FCreateTextureWithStringSignature OnFail;
	
	void ScreenCaptureWithStringCompleted(int32 InWidth, int32 InHeight, const TArray<FColor>& InColor);

private:
	FString Options = "";
	FDelegateHandle CaptureWithStringHandle;
};