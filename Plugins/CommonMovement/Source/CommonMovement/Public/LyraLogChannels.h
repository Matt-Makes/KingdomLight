// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "Logging/LogMacros.h"

class UObject;

COMMONMOVEMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
COMMONMOVEMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraExperience, Log, All);
COMMONMOVEMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraAbilitySystem, Log, All);
COMMONMOVEMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraTeams, Log, All);


COMMONMOVEMENT_API DECLARE_LOG_CATEGORY_EXTERN(LogCMC, Log, All);

// Judge a UObject is on the server side or on the client side.
COMMONMOVEMENT_API FString GetClientServerContextString(UObject* ContextObject = nullptr);


static void LogOnScreen(UObject* WorldContext, FString Msg, FColor Color = FColor::White, float Duration = 5.0f)
{
	if (!ensure(WorldContext))
	{
		return;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FString NetPrefix = World->IsNetMode(NM_Client) ? "[CLIENT] " : "[SERVER] ";
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, NetPrefix + Msg);
	}
}