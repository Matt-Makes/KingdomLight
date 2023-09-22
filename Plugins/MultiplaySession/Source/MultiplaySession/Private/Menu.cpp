// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "Components/Button.h"
#include "MultiplaySessionsSubsystem.h"
#include "OnlineSessionSettings.h"
//#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"


void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString MatchOfType, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = MatchOfType;
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	//bIsFocusable = true;
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if(PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget()); //共享指针也接受共享引用；但共享引用只接受共享引用
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	// UI添加到屏幕之后，再缓存
	UGameInstance* GI = GetGameInstance();
	if(GI)
	{
		MultiplaySessionsSubsystem = GI->GetSubsystem<UMultiplaySessionsSubsystem>(); //如何拿到缓存，这一步很重要
	}

	if(MultiplaySessionsSubsystem)
	{
		MultiplaySessionsSubsystem->MultiplaySS_CreateSessionDelegate.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplaySessionsSubsystem->MultiplaySS_FindSessionDelegate.AddUObject(this, &ThisClass::OnFindSession);
		MultiplaySessionsSubsystem->MultiplaySS_JoinSessionDelegate.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplaySessionsSubsystem->MultiplaySS_DestroySessionDelegate.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplaySessionsSubsystem->MultiplaySS_StartSessionDelegate.AddDynamic(this, &ThisClass::OnStartSession);
	}
}



bool UMenu::Initialize()
{
	// 先调用父类
	if(!Super::Initialize())
	{
		return false;
	}
	
	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked); //动态多播(绑定至 FDelegate或 FEvent)
	}
	if(JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	
	return true;
}

// Widgets added to the viewport are automatically removed if the persistent level is unloaded.
void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}
// void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
// {
// 	MenuTearDown();
// 	Super::OnLevelRemovedFromWorld(InLevel, InWorld); //调用父类 保险措施
// }

// 帮别人干活（A的事交给B来做）
void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("Session created successfully!"))
				);
		}
		/** 会话创建完成了，才开始世界旅行 */
		UWorld* World = GetWorld();
		if(World)
		{
			World->ServerTravel(PathToLobby); //参数为 统一资源定位器
		}
	}
	else
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed created sessions"))
				);
		}

		// 没有成功创建
		HostButton->SetIsEnabled(true);
		JoinButton->SetIsEnabled(true);
	}
}

//TODO Find终点。
void UMenu::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if(MultiplaySessionsSubsystem == nullptr)
	{
		return;
	}
	
	for(auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if(SettingsValue == MatchType)
		{
			/** TODO  Join起点 */
			MultiplaySessionsSubsystem->JoinSession(Result);
			return; //打断
		}
	}
	// 没有找到
	if(!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

// 先获取到OnlineSessionInterface，拿到ip地址；再拿到PlayerController，去ClientTravel
void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		IOnlineSessionPtr OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
		if(OnlineSessionInterface.IsValid())
		{
			FString Address; //URL,也就是一个ip
			OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
			
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if(PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	// 找到了，但没有成功加入
	if(Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}


void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	
	if(MultiplaySessionsSubsystem)
	{
		MultiplaySessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
		// UWorld* World = GetWorld();
		// if(World)
		// {
		// 	World->ServerTravel("/Game/ThirdPersonCPP/Maps/Lobby?listen"); //参数为 统一资源定位器
		// }
	}
}

//TODO Find起点。  先Find，再Join
void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	
	if(MultiplaySessionsSubsystem)
	{
		MultiplaySessionsSubsystem->FindSessions(10000);
	}
}

// 移除菜单UI
void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		FInputModeGameOnly InputModeData;
		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(false);
	}
}
