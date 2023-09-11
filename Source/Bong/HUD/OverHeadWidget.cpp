// Fill out your copyright notice in the Description page of Project Settings.


#include "OverHeadWidget.h"
#include "Components/TextBlock.h"


UOverHeadWidget::UOverHeadWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UOverHeadWidget::SetDisplayText(FString TextToDisplay)
{
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverHeadWidget::ShowPlayerLocalRole(APawn* InPawn)
{
	// Pawn无效会崩引擎
	if(InPawn)
	{
		ENetRole LocalRole = InPawn->GetLocalRole();
        FString Role;
        
        switch(LocalRole)
        {
        case ENetRole::ROLE_Authority:
        	Role = FString("Authority");
        	break;
        case ENetRole::ROLE_AutonomousProxy:
        	Role = FString("Autonomous Proxy");
        	break;
        case ENetRole::ROLE_SimulatedProxy:
        	Role = FString("Simulated Proxy");
        	break;
        case ENetRole::ROLE_None:
        	Role = FString("None");
        	break;
        }
        FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
    
        SetDisplayText(LocalRoleString);
	}
	
}

void UOverHeadWidget::ShowPlayerRemoteRole(APawn* InPawn)
{
	if(InPawn)
	{
		ENetRole RemoteRole = InPawn->GetRemoteRole();
        FString Role;
        
        switch(RemoteRole)
        {
        case ENetRole::ROLE_Authority:
        	Role = FString("Authority");
        	break;
        case ENetRole::ROLE_AutonomousProxy:
        	Role = FString("Autonomous Proxy");
        	break;
        case ENetRole::ROLE_SimulatedProxy:
        	Role = FString("Simulated Proxy");
        	break;
        case ENetRole::ROLE_None:
        	Role = FString("None");
        	break;
        }
        FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
    
        SetDisplayText(RemoteRoleString);
	}
	
}

void UOverHeadWidget::NativeDestruct()
{
	RemoveFromParent(); //避免报错
	Super::NativeDestruct();
}
// void UOverHeadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
// {
// 	RemoveFromParent(); //避免报错
// 	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
// }
