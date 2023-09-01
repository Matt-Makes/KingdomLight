// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BongHUD.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(BongHUD, Log, All);


class UUserWidget;
class UCharacterOverlay;
class UAnnouncement;
class UTexture2D;


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Transient)
	UTexture2D* CrosshairsCenter = nullptr;
	UPROPERTY(Transient)
	UTexture2D* CrosshairsTop = nullptr;
	UPROPERTY(Transient)
	UTexture2D* CrosshairsBottom = nullptr;
	UPROPERTY(Transient)
	UTexture2D* CrosshairsLeft = nullptr;
	UPROPERTY(Transient)
	UTexture2D* CrosshairsRight = nullptr;

	float CrosshairsSpread;
	FLinearColor CrosshairsColor;
};

UCLASS()
class BONG_API ABongHUD : public AHUD
{
	GENERATED_BODY()

public:
	ABongHUD(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// 会自动每帧调用
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	UPROPERTY(EditAnywhere, Category = "Announcement")
	TSubclassOf<UUserWidget> AnnouncementClass;
	
	// 存储创建的UI
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
	UPROPERTY()
	UAnnouncement* Announcement;
	
	void AddCharacterOverlay();
	void AddAnnouncement();
	
protected:
	virtual void BeginPlay() override;
	

	
private:
	FHUDPackage HUDPackage;
	void DrawCrosshairs(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor);

	UPROPERTY(EditAnywhere)
	float CrosshairsSpreadMax = 16.f;

	
public:
	// 引用传参，绑定外部值，实时改变
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
	
};

