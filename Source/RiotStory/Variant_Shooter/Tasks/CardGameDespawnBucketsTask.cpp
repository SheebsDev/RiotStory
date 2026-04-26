#include "CardGameDespawnBucketsTask.h"

#include "RiotStory.h"
#include "Variant_Shooter/CardBucketDirector.h"
#include "Variant_Shooter/ShooterGameMode.h"
#include "Variant_Shooter/ShooterPlayerController.h"

void UCardGameDespawnBucketsTask::Initialize(FCardGameSetupTaskContext* InOutContext)
{
    SetupContext = InOutContext;
}

void UCardGameDespawnBucketsTask::Activate()
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
        FailTask(TEXT("Could not resolve world for bucket despawn."));
        return;
    }

    AShooterGameMode* const ShooterGameMode = Cast<AShooterGameMode>(World->GetAuthGameMode());
    ACardBucketDirector* const BucketDirector = ShooterGameMode->GetCardBucketDirector();

    BucketDirector->DespawnAllBuckets();
    ShooterGameMode->DestroyCardBucketDirector();
    bWasSuccessful = true;
    EndTask();
}

void UCardGameDespawnBucketsTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
