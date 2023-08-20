// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureToCreateTexture2D.h"
#include "IImageWrapper.h"
#include "ImageUtils.h"


UCaptureToCreateTexture* UCaptureToCreateTexture::CaptureToCreateTexture(UObject* WorldContextObject, bool bCaptureUI)
{
	UCaptureToCreateTexture* CaptureTask = NewObject<UCaptureToCreateTexture>();

	CaptureTask->CaptureHandle = UGameViewportClient::OnScreenshotCaptured().AddUObject(CaptureTask, &UCaptureToCreateTexture::ScreenCaptureCompleted);
	if(bCaptureUI)
	{
		FString Resolution = FString::Printf(TEXT("Shot ShowUI"));
		GEngine->GameViewport->Exec(WorldContextObject->GetWorld(), *Resolution, *GLog);
	}
	else
	{
		FString Resolution = FString::Printf(TEXT("Shot"));
		GEngine->GameViewport->Exec(WorldContextObject->GetWorld(), *Resolution, *GLog);
	}
	
	return CaptureTask;
}


void UCaptureToCreateTexture::ScreenCaptureCompleted(int32 InWidth, int32 InHeight, const TArray<FColor>& InColor)
{
	bool bSuccess = false;
	UTexture2D* NewTexture = nullptr;
	NewTexture = UTexture2D::CreateTransient(InWidth, InHeight, PF_B8G8R8A8); //CreateTransient相当于创造一个空间。

	if (NewTexture)
	{
		void* TextureData = NewTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE); //多线程时不加锁，内存拷贝是会崩溃的。
		FMemory::Memcpy(TextureData, InColor.GetData(), InColor.GetAllocatedSize()); //(目标，源，数量或大小)。
		NewTexture->PlatformData->Mips[0].BulkData.Unlock();
		NewTexture->UpdateResource();

		bSuccess = true;
	}
	
	if(bSuccess)
	{
		OnSuccess.Broadcast(NewTexture);
	}
	else
	{
		OnFail.Broadcast(nullptr);
	}

	//移除句柄。
	UGameViewportClient::OnScreenshotCaptured().Remove(CaptureHandle);
}
