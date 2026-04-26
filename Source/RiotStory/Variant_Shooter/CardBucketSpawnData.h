#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CardBucketSpawnData.generated.h"

class ACardBucket;

USTRUCT(BlueprintType)
struct RIOTSTORY_API FCardBucketSpawnRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Identifier used to group rows into a spawn set. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket")
    FName SetId = NAME_None;

    /** Spawn point identifier that this row targets in-level. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket")
    FName SpawnPointId = NAME_None;
};

