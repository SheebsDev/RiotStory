#include "CardGameSpawnBucketsTask.h"

#include "Variant_Shooter/CardBucketDirector.h"
#include "Variant_Shooter/ShooterGameMode.h"
#include "Variant_Shooter/ShooterPlayerController.h"

void UCardGameSpawnBucketsTask::Initialize(FCardGameSetupTaskContext* InOutContext)
{
    SetupContext = InOutContext;
}

void UCardGameSpawnBucketsTask::Activate()
{
    bWasSuccessful = false;
    FailureReason.Reset();

    if (!SetupContext || !SetupContext->IsValidForSwap())
    {
        FailTask(TEXT("Setup context is null or missing swap references."));
        return;
    }

    AShooterPlayerController* const ShooterController = SetupContext->ShooterController.Get();
    UWorld* const World = ShooterController->GetWorld();
    if (!IsValid(World))
    {
        FailTask(TEXT("Could not resolve world for bucket spawn."));
        return;
    }

    AShooterGameMode* const ShooterGameMode = Cast<AShooterGameMode>(World->GetAuthGameMode());
    ACardBucketDirector* const BucketDirector = ShooterGameMode->CreateCardBucketDirector();

    FString SpawnFailureReason;
    if (!BucketDirector->SpawnActiveSet(SpawnFailureReason))
    {
        ShooterGameMode->DestroyCardBucketDirector();
        FailTask(FString::Printf(TEXT("Card bucket spawn failed: %s"), *SpawnFailureReason));
        return;
    }

    bWasSuccessful = true;
    EndTask();
}

void UCardGameSpawnBucketsTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
