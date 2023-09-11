// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;

UCLASS()
class BONG_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()
public:
	UAnnouncement(const FObjectInitializer& ObjectInitializer);

	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarmupTimeText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InfoText;

	
};
