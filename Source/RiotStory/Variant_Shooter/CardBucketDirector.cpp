#include "Variant_Shooter/CardBucketDirector.h"

#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "RiotStory.h"
#include "Variant_Shooter/CardBucket.h"
#include "Variant_Shooter/CardBucketSpawnData.h"
#include "Variant_Shooter/CardBucketSpawnPoint.h"

ACardBucketDirector::ACardBucketDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACardBucketDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DespawnAllBuckets();
    Super::EndPlay(EndPlayReason);
}

bool ACardBucketDirector::BuildSpawnPointMap(TMap<FName, ACardBucketSpawnPoint*>& OutSpawnPoints, FString& OutFailureReason) const
{
    OutSpawnPoints.Reset();
    OutFailureReason.Reset();

    UWorld* const World = GetWorld();
    if (!IsValid(World))
    {
        OutFailureReason = TEXT("Bucket director has no valid world.");
        return false;
    }

    TArray<AActor*> SpawnPointActors;
    UGameplayStatics::GetAllActorsOfClass(World, ACardBucketSpawnPoint::StaticClass(), SpawnPointActors);

    for (AActor* const SpawnPointActor : SpawnPointActors)
    {
        ACardBucketSpawnPoint* const SpawnPoint = Cast<ACardBucketSpawnPoint>(SpawnPointActor);
        if (!IsValid(SpawnPoint))
        {
            continue;
        }

        if (SpawnPoint->SpawnPointId.IsNone())
        {
            UE_LOG(LogRiotStory, Warning, TEXT("CardBucketSpawnPoint '%s' has no SpawnPointId and will be ignored."), *SpawnPoint->GetName());
            continue;
        }

        if (OutSpawnPoints.Contains(SpawnPoint->SpawnPointId))
        {
            OutFailureReason = FString::Printf(TEXT("Duplicate CardBucketSpawnPoint id '%s' in level."), *SpawnPoint->SpawnPointId.ToString());
            return false;
        }

        OutSpawnPoints.Add(SpawnPoint->SpawnPointId, SpawnPoint);
    }

    return true;
}

bool ACardBucketDirector::SpawnActiveSet(FString& OutFailureReason)
{
    OutFailureReason.Reset();
    DespawnAllBuckets();

    if (!IsValid(BucketSpawnTable))
    {
        OutFailureReason = TEXT("BucketSpawnTable is not configured on CardBucketDirector.");
        return false;
    }

    if (ActiveSetId.IsNone())
    {
        OutFailureReason = TEXT("ActiveSetId is not configured on CardBucketDirector.");
        return false;
    }

    TMap<FName, ACardBucketSpawnPoint*> SpawnPointsById;
    if (!BuildSpawnPointMap(SpawnPointsById, OutFailureReason))
    {
        return false;
    }

    static const FString TableContext(TEXT("CardBucketDirector::SpawnActiveSet"));
    TArray<FCardBucketSpawnRow*> AllRows;
    BucketSpawnTable->GetAllRows(TableContext, AllRows);

    TArray<FCardBucketSpawnRow*> SetRows;
    SetRows.Reserve(AllRows.Num());
    for (FCardBucketSpawnRow* const Row : AllRows)
    {
        if (Row && Row->SetId == ActiveSetId)
        {
            SetRows.Add(Row);
        }
    }

    if (SetRows.Num() == 0)
    {
        OutFailureReason = FString::Printf(TEXT("No spawn rows found for ActiveSetId '%s'."), *ActiveSetId.ToString());
        return false;
    }

    for (const FCardBucketSpawnRow* const Row : SetRows)
    {
        if (Row->SpawnPointId.IsNone())
        {
            OutFailureReason = FString::Printf(TEXT("Set '%s' contains a row with no SpawnPointId."), *ActiveSetId.ToString());
            return false;
        }

        if (!SpawnPointsById.Contains(Row->SpawnPointId))
        {
            OutFailureReason = FString::Printf(TEXT("Set '%s' row references missing SpawnPointId '%s'."), *ActiveSetId.ToString(), *Row->SpawnPointId.ToString());
            return false;
        }

        const ACardBucketSpawnPoint* const SpawnPoint = SpawnPointsById.FindRef(Row->SpawnPointId);
        if (!IsValid(SpawnPoint))
        {
            OutFailureReason = FString::Printf(TEXT("Set '%s' row resolved invalid SpawnPointId '%s'."), *ActiveSetId.ToString(), *Row->SpawnPointId.ToString());
            return false;
        }

        if (!SpawnPoint->BucketClass)
        {
            OutFailureReason = FString::Printf(TEXT("Spawn point '%s' has no BucketClass configured."), *Row->SpawnPointId.ToString());
            return false;
        }
    }

    UWorld* const World = GetWorld();
    if (!IsValid(World))
    {
        OutFailureReason = TEXT("Bucket director has no valid world during spawn.");
        return false;
    }

    for (const FCardBucketSpawnRow* const Row : SetRows)
    {
        ACardBucketSpawnPoint* const SpawnPoint = SpawnPointsById.FindRef(Row->SpawnPointId);
        check(SpawnPoint);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ACardBucket* const SpawnedBucket = World->SpawnActor<ACardBucket>(SpawnPoint->BucketClass, SpawnPoint->GetActorTransform(), SpawnParams);
        if (!IsValid(SpawnedBucket))
        {
            OutFailureReason = FString::Printf(TEXT("Failed to spawn bucket at SpawnPointId '%s'."), *Row->SpawnPointId.ToString());
            DespawnAllBuckets();
            return false;
        }

        const FTransform SpawnTransform = SpawnPoint->GetActorTransform();
        const FVector StartWorld = SpawnTransform.TransformPosition(SpawnPoint->StartOffset);
        const FVector EndWorld = SpawnTransform.TransformPosition(SpawnPoint->EndOffset);

        SpawnedBucket->StartPosition = StartWorld;
        SpawnedBucket->EndPosition = EndWorld;
        SpawnedBucket->CycleSpeed = FMath::Max(0.f, SpawnPoint->CycleSpeed);
        SpawnedBucket->bInfinite = SpawnPoint->bInfinite;
        SpawnedBucket->CycleDelay = FMath::Max(0.f, SpawnPoint->CycleDelay);
        SpawnedBucket->CyclePosition = 0.f;
        SpawnedBucket->bReturnToStart = false;
        SpawnedBucket->SetActorLocation(StartWorld);

        const bool bShouldBeginMoving = SpawnPoint->bOverrideBeginMoving ? SpawnPoint->bBeginMoving : bBeginMovingOnSpawn;
        if (bShouldBeginMoving)
        {
            SpawnedBucket->BeginMoving();
        }
        else
        {
            SpawnedBucket->StopMoving();
        }

        SpawnedBuckets.Add(SpawnedBucket);
    }

    return true;
}

void ACardBucketDirector::DespawnAllBuckets()
{
    TArray<TWeakObjectPtr<ACardBucket>> BucketsToDestroy = MoveTemp(SpawnedBuckets);
    SpawnedBuckets.Reset();

    for (const TWeakObjectPtr<ACardBucket>& Bucket : BucketsToDestroy)
    {
        if (!Bucket.IsValid())
        {
            continue;
        }

        if (!Bucket->IsActorBeingDestroyed())
        {
            Bucket->Destroy();
        }
    }
}
