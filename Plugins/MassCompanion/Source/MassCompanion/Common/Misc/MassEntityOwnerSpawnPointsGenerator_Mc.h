// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySpawnDataGeneratorBase.h"
#include "MassSpawnLocationProcessor.h"
#include "MassSpawnerTypes.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityOwnerSpawnPointsGenerator_Mc.generated.h"

/**
 * Describes the SpawnPoints Generator when we want to spawn directly on owner location.
 */
UCLASS(BlueprintType, meta=(DisplayName="Owner SpawnPoints Generator Mc"))
class MASSCOMPANION_API UMassEntityOwnerSpawnPointsGenerator_Mc : public UMassEntitySpawnDataGeneratorBase
{
	GENERATED_BODY()
	
public:
	virtual void Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count, FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const override
	{
		const FTransform& OwnerTransform = CastChecked<AActor>(&QueryOwner)->GetTransform();

		if (Count <= 0 || !OwnerTransform.IsValid())
		{
			FinishedGeneratingSpawnPointsDelegate.Execute(TArray<FMassEntitySpawnDataGeneratorResult>());
			return;
		}

		
		float TotalProportion = 0.0f;
		for (const FMassSpawnedEntityType& EntityType : EntityTypes)
		{
			TotalProportion += EntityType.Proportion;
		}
		
		if (TotalProportion <= 0)
		{
			// I need an adult... why does (EntityCount > 0 && EntityType.GetEntityConfig() != nullptr) linker error?
			// oh, it's just not exported... 
			//UE_VLOG_UELOG(this, LogMassSpawner, Error, TEXT("The total combined proportion of all the entity types needs to be greater than 0."));
			return;
		}

		// for (int32 i = 0; i < EntityTypes.Num(); i++)
		// {
		// 	const FMassSpawnedEntityType& EntityType = EntityTypes[i];
		// 	const int32 EntityCount = int32(Count * EntityType.Proportion / TotalProportion);
		// 	if (EntityCount > 0 && EntityType.GetEntityConfig() != nullptr)
		// 	{
		// 		FMassEntitySpawnDataGeneratorResult& Res = OutResults.AddDefaulted_GetRef();
		// 		Res.NumEntities = EntityCount;
		// 		Res.EntityConfigIndex = i;
		// 	}
		// }
		
		
		TArray<FMassEntitySpawnDataGeneratorResult> Results;
		
		for (int32 i = 0; i < EntityTypes.Num(); i++)
		{
			const FMassSpawnedEntityType& EntityType = EntityTypes[i];
			const int32 EntityCount = int32(Count * EntityType.Proportion / TotalProportion);

			// I need an adult... why does (EntityCount > 0 && EntityType.GetEntityConfig() != nullptr) linker error?
			// oh, it's just not exported... 
			if (EntityCount > 0/* && EntityType.GetEntityConfig() != nullptr*/)
			{
				FMassEntitySpawnDataGeneratorResult& Res = Results.AddDefaulted_GetRef();
				Res.NumEntities = EntityCount;
				Res.EntityConfigIndex = i;
				Res.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
				
				Res.SpawnData.InitializeAs<FMassTransformsSpawnData>();
				FMassTransformsSpawnData& Transforms = Res.SpawnData.GetMutable<FMassTransformsSpawnData>();
				Transforms.Transforms.Reserve(Res.NumEntities);
				
				int32 v = 0;
				while (v < EntityCount)
				{
					FTransform& Transform = Transforms.Transforms.AddDefaulted_GetRef();
					Transform = OwnerTransform;
					v++;
				}
			}
		}
		
		FinishedGeneratingSpawnPointsDelegate.Execute(Results);
	}
	
};