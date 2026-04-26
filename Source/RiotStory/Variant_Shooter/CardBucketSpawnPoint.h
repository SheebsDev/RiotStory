#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardBucketSpawnPoint.generated.h"

class ACardBucket;
class USceneComponent;

UCLASS()
class RIOTSTORY_API ACardBucketSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    ACardBucketSpawnPoint();

    /** Stable id used by bucket spawn table rows to target this point. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket")
    FName SpawnPointId = NAME_None;

    /** Bucket class to spawn from this point */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket")
    TSubclassOf<ACardBucket> BucketClass;

    /** Offset from spawn point transform used for bucket StartPosition. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    FVector StartOffset = FVector::ZeroVector;

    /** Offset from spawn point transform used for bucket EndPosition. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    FVector EndOffset = FVector::ZeroVector;

    /** Override speed for bucket cycle movement. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    float CycleSpeed = 1.f;

    /** Override infinite back-and-forth behavior. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    bool bInfinite = true;

    /** Override cycle delay. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    float CycleDelay = 0.f;

    /** When true, bBeginMoving is used instead of director default behavior. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    bool bOverrideBeginMoving = false;

    /** Optional per-point start moving behavior when bOverrideBeginMoving is true. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    bool bBeginMoving = true;
};
