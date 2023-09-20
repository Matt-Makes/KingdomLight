#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FQuestInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct BLUEPRINTTOCPP_API FQuestInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Progress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProgressTotal;
};
