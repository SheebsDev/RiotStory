#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardBucketDirector.generated.h"

class ACardBucket;
class ACardBucketSpawnPoint;
class UDataTable;

UCLASS()
class RIOTSTORY_API ACardBucketDirector : public AActor
{
    GENERATED_BODY()

public:
    ACardBucketDirector();

    /** Spawns all rows that match ActiveSetId from BucketSpawnTable. */
    UFUNCTION(BlueprintCallable, Category="Card Buckets")
    bool SpawnActiveSet(FString& OutFailureReason);

    /** Destroys all buckets currently spawned by this director. */
    UFUNCTION(BlueprintCallable, Category="Card Buckets")
    void DespawnAllBuckets();

    UFUNCTION(BlueprintCallable, Category="Card Buckets")
    void SetActiveSetId(const FName InSetId) { ActiveSetId = InSetId; }

    UFUNCTION(BlueprintPure, Category="Card Buckets")
    FName GetActiveSetId() const { return ActiveSetId; }

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Table containing set-based card bucket spawn rows. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Buckets")
    TObjectPtr<UDataTable> BucketSpawnTable = nullptr;

    /** The currently selected set id to spawn when SpawnActiveSet is called. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Buckets")
    FName ActiveSetId = NAME_None;

private:
    TArray<TWeakObjectPtr<ACardBucket>> SpawnedBuckets;

    bool BuildSpawnPointMap(TMap<FName, ACardBucketSpawnPoint*>& OutSpawnPoints, FString& OutFailureReason) const;
};
