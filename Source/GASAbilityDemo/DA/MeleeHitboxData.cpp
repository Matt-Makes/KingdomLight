// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeHitboxData.h"

// GAS 有蓝图版 本地预测，可能很方便，不用像 Stephen那么耦合
TArray<FMeleeHitSphereDefinition> UMeleeHitboxData::GetMeleeHitSpheres(TArray<int> indexes)
{

	TArray<FMeleeHitSphereDefinition> hitSphereSubset;

	for (int i = 0; i < indexes.Num(); i++)
	{
		int currentIndex = indexes[i];

		if (MeleeHitSpheres.Num() > currentIndex && currentIndex >= 0)
		{
			hitSphereSubset.Add(MeleeHitSpheres[currentIndex]);
		}
	}

	return hitSphereSubset;
}
