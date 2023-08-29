#pragma once

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWY_AssaultRifle		UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher		UMETA(DisplayName = "Rocket Launcher"),
	EWT_GrenadeLauncher		UMETA(DisplayName = "Grenade Launcher"),
	
	EWT_Pistol				UMETA(DisplayName = "Pistol"),
	EWY_SMG					UMETA(DisplayName = "SMG"),
	EWY_Shotgun				UMETA(DisplayName = "Shotgun"),
	EWY_SniperRifle			UMETA(DisplayName = "Sniper Rifle"),

	EWY_MAX					UMETA(DisplayName = "DefaultMax")
};
