#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied			UMETA(DisplayName = "Unoccupied"),/* 在这个状态下，才能去做其他事 */
	ECS_Reloading			UMETA(DisplayName = "Reloading"),
	ECS_ThrowingGrenade		UMETA(DisplayName = "Throwing Grenade"),
	ECS_SwappingWeapons		UMETA(DisplayName = "Swapping Weapons"),

	ECS_MAX					UMETA(DisplayName = "DefaultMAX")
};