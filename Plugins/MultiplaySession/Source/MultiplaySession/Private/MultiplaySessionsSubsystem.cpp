// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplaySessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
//#include "Interfaces/OnlineSessionInterface.h"

UMultiplaySessionsSubsystem::UMultiplaySessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if(OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
	}
}


void UMultiplaySessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if(!SessionInterface.IsValid())
	{
		return;
	}
	
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if(ExistingSession != nullptr)
	{
		// 这里一般不执行，若执行，在销毁Session时，立马接下文的SessionInterface->CreateSession有问题
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		DestroySession();
	}
	
	// Store the delegate in a FDelegateHandle so we can later remove it from delegate list.
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings()); //初始化，初始化
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; //返回URL 或 NULL; 也返回Steam，steam不是局域网
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true; //使用存在感信息寻找世界上我们地区正在进行的Session
	
	// 创建Session时提供的信息，他人寻找Session时会Check以确保正确
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1; //找房时，可以看到很多玩家的自建房，否则只能看见第一个自建房
	LastSessionSettings->bUseLobbiesIfAvailable = true; //UE5 正式版才有; Whether to prefer lobbies APIs if the platform supports them

	//OnDestroySessionComplete
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if( !SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle); //创建成功或失败都要Clear

		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString(TEXT("Donnnnnnnnn't connect steam, CreateSession failed "))
			);
		
		// Broadcast out own custom delegate
		MultiplaySS_CreateSessionDelegate.Broadcast(false);
	}
}

void UMultiplaySessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if(!SessionInterface.IsValid())
	{
		return;
	}
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch()); //初始化
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; //照顾局域网和连接到Steam时这两种情况
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if( !SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()) )
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		// 创建一个空数组作为参数
		MultiplaySS_FindSessionDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
	
}

void UMultiplaySessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	if( !SessionInterface.IsValid())
	{
		MultiplaySS_JoinSessionDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError); //对未知错误广播
		return;
	}
	
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if( !SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult) )
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplaySS_JoinSessionDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
	
}

// 类的自定义的成员函数
void UMultiplaySessionsSubsystem::DestroySession()
{
	if(!SessionInterface.IsValid())
	{
		MultiplaySS_DestroySessionDelegate.Broadcast(false);
		return;
	}
	// TODO 1加入委托列表，委托成功Fire，其会在回调函数里清除
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	
	if( !SessionInterface->DestroySession(NAME_GameSession) )
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplaySS_DestroySessionDelegate.Broadcast(false);
	}
}

void UMultiplaySessionsSubsystem::StartSession()
{
	if(!SessionInterface.IsValid())
	{
		MultiplaySS_StartSessionDelegate.Broadcast(false);
		return;
	}

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
	
	if( !SessionInterface->StartSession(NAME_GameSession) )
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MultiplaySS_StartSessionDelegate.Broadcast(false);
	}
}







// Steam返回信息后，SessionInterface先迭代自己的委托列表，触发相应委托执行这个函数；然后我们再触发自己的自定义委托的广播
void UMultiplaySessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle); //创建成功就Clear
	}
	
	// Broadcast out own custom delegate
	MultiplaySS_CreateSessionDelegate.Broadcast(bWasSuccessful);
}

// SessionInterface->FindSessions() 若执行成功，这个在构造时绑好的回调函数会执行
void UMultiplaySessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if(SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
	if(LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplaySS_FindSessionDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	
	MultiplaySS_FindSessionDelegate.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplaySessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if(SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	
	MultiplaySS_JoinSessionDelegate.Broadcast(Result);
}

// TODO 2绑定在Fired的委托上 的回调函数
void UMultiplaySessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if(SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if(bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		// TODO 3创建是在这里
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplaySS_DestroySessionDelegate.Broadcast(bWasSuccessful); //广播以告知 Menu
}

void UMultiplaySessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}




























