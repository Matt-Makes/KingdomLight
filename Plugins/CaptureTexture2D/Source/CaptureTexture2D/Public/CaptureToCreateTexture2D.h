// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "UObject/Object.h"
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

	/*返回一个GetWorld，上下文世界环境。*/
public:                         
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = "ScreenCapture")
	static UCaptureToCreateTexture* CaptureToCreateTexture(UObject* WorldContextObject, bool bCaptureUI = true);

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