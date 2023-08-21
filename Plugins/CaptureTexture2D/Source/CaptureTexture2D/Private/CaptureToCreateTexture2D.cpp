// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureToCreateTexture2D.h"
#include "IImageWrapper.h"
//#include "IImageWrapperModule.h"
#include "ImageUtils.h"


UCaptureToCreateTexture* UCaptureToCreateTexture::CaptureToCreateTexture(UObject* WorldContextObject, bool bCaptureUI)
{
	//********不是void就得有一个返回值，一个重要实例，对象。定义这个类自身的局部变量。
	UCaptureToCreateTexture* CaptureTask = NewObject<UCaptureToCreateTexture>();

	//绑定截图的代理：绑定回调到 CaptureScreenCompleted函数。********类-游戏窗口代理人，绑定一个UObject类型(绑定到哪一个对象，绑定到哪一个成员函数).
	CaptureTask->CaptureHandle = UGameViewportClient::OnScreenshotCaptured().AddUObject(CaptureTask, &UCaptureToCreateTexture::ScreenCaptureCompleted);
	//开始执行截图命令函数。********(上下文环境世界，命令行字符串，log日志（全局Log单例）).
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
	/*编辑器能过，打包能过，可能是性能最好 */
	bool bSuccess = false;
	UTexture2D* NewTexture = nullptr;
	NewTexture = UTexture2D::CreateTransient(InWidth, InHeight, PF_B8G8R8A8); //CreateTransient相当于创造一个临时空间。

	if (NewTexture)
	{
		// TextureData 指向上锁的图片资源
		void* TextureData = NewTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE); //多线程时不加锁，内存拷贝是会崩溃的。
		FMemory::Memcpy(TextureData, InColor.GetData(), InColor.GetAllocatedSize()); //(目标，源，数量或大小)。
		NewTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
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
