// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplaySessionsSubsystem.generated.h"


// Declaring our own custom delegates for the Menu class to bind callback to.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplaySS_OnCreateSessionDelegate, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplaySS_OnFindSessionDelegate, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplaySS_OnJoinSessionDelegate, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplaySS_OnDestroySessionDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplaySS_OnStartSessionDelegate, bool, bWasSuccessful);


UCLASS()
class MULTIPLAYSESSION_API UMultiplaySessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UMultiplaySessionsSubsystem();

	/** To handle session functionality. The Menu class will call these outside */
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);
	void DestroySession();
	void StartSession();

	/** Our own custom delegates for the Menu class to bind callbacks to. */
	FMultiplaySS_OnCreateSessionDelegate MultiplaySS_CreateSessionDelegate;
	FMultiplaySS_OnFindSessionDelegate MultiplaySS_FindSessionDelegate;
	FMultiplaySS_OnJoinSessionDelegate MultiplaySS_JoinSessionDelegate;
	FMultiplaySS_OnDestroySessionDelegate MultiplaySS_DestroySessionDelegate;
	FMultiplaySS_OnStartSessionDelegate MultiplaySS_StartSessionDelegate;
	
protected:
	/*
	 * Internal Callback functions for the delegates that we'll add to OnlineSessionInterface's delegate list.
	 * These don't need to be called outside this class.
	 */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	
private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings; //CreateSession时初始化
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch; //FindSession时初始化

	
	/*
	 * To add to the OnlineSessionInterface's delegate list.
	 * We'll bind our MultiplaySessionSubsystem internal Callback functions to these
	 */
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle; //句柄的作用，缓存 添加到委托列表时的返回值，用于Clear，或者，消耗输入
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
};
