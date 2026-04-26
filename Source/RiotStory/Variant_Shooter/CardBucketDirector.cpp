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

        checkf(!SpawnPoint->SpawnPointId.IsNone(), TEXT("Spawn Point Id was not set on the level actor"));

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

    checkf(IsValid(BucketSpawnTable), TEXT("BucketSpawnTable is not configured in blueprints for CardBucketDirector"));
    checkf(!ActiveSetId.IsNone(), TEXT("ActiveSetId is not configured in blueprints for CardBucketDirector"));

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

    //TODO: Remove or find a better place to validate instead of silently failing here
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

        const TArray<FVector> WorldControlPoints = SpawnPoint->ControlPointOffsets;
        if (WorldControlPoints.Num() < 2)
        {
            OutFailureReason = FString::Printf(TEXT("Spawn point '%s' must define at least 2 ControlPointOffsets."), *Row->SpawnPointId.ToString());
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
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ACardBucket* const SpawnedBucket = World->SpawnActor<ACardBucket>(SpawnPoint->BucketClass, SpawnPoint->GetActorTransform(), SpawnParams);
        if (!IsValid(SpawnedBucket))
        {
            OutFailureReason = FString::Printf(TEXT("Failed to spawn bucket at SpawnPointId '%s'."), *Row->SpawnPointId.ToString());
            DespawnAllBuckets();
            return false;
        }

        SpawnedBucket->CycleSpeed = FMath::Max(0.f, SpawnPoint->CycleSpeed);
        SpawnedBucket->bInfinite = SpawnPoint->bInfinite;
        SpawnedBucket->SetControlPointOffsets(SpawnPoint->ControlPointOffsets);

        const bool bShouldBeginMoving = SpawnPoint->bBeginMoving;
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
    for (const TWeakObjectPtr<ACardBucket>& Bucket : SpawnedBuckets)
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
