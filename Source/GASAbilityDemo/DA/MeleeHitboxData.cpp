// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeHitboxData.h"

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
