// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassRepresentationProcessor.h"
#include "MassVisualizationLODProcessor.h"

#include "MassVisualizationProcessor_McSample.generated.h"


/**
 * Example processors demonstrating how to use the representation module.
 */
UCLASS()
class MASSCOMPANION_API UMassVisualizationProcessor_McSample : public UMassVisualizationProcessor
{
	GENERATED_BODY()

public:

	UMassVisualizationProcessor_McSample()
	{
		// Configures whether this processor should be automatically included in the global list of processors executed every tick (see ProcessingPhase and ExecutionOrder).
		bAutoRegisterWithProcessingPhases = true;
	};

};

UCLASS()
class MASSCOMPANION_API UMassVisualizationLODProcessor_McSample : public UMassVisualizationLODProcessor
{
	GENERATED_BODY()

public:

	UMassVisualizationLODProcessor_McSample()
	{
		bAutoRegisterWithProcessingPhases = true;
	};

};