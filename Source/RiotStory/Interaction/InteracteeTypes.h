#pragma once

#include "CoreMinimal.h"
#include "InteracteeTypes.generated.h"

USTRUCT(BlueprintType)
struct RIOTSTORY_API FInteractionDisplayInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText ActionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FText DisplayName;
};

