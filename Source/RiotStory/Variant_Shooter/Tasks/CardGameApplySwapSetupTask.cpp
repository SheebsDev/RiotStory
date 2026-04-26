#include "Variant_Shooter/Tasks/CardGameApplySwapSetupTask.h"

#include "UI/CardGameUI.h"
#include "Variant_Shooter/CardGamePlayerStartPoint.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Variant_Shooter/Tasks/CardGameSpawnBucketsTask.h"

void UCardGameApplySwapSetupTask::Initialize(FCardGameSetupTaskContext* InOutContext)
{
    SetupContext = InOutContext;
}

void UCardGameApplySwapSetupTask::Activate()
{
    bWasSuccessful = false;
    FailureReason.Reset();

    if (!SetupContext || !SetupContext->IsValidForSwap())
    {
        FailTask(TEXT("Setup context is null or invalid"));
        return;
    }

    AShooterPlayerController* const ShooterController = SetupContext->ShooterController.Get();
    AShooterCharacter* const ShooterPawn = SetupContext->ShooterPawn.Get();
    ACardGamePlayerStartPoint* const StartPoint = SetupContext->CardGameStartPoint.Get();

    SetupContext->PlayerReturnTransform = ShooterPawn->GetActorTransform();
    ShooterPawn->SetActorLocationAndRotation(StartPoint->GetActorLocation(), StartPoint->GetActorRotation());

    UCardGameUI* const CardGameUI = ShooterController->GetOrCreateCardGameUI();
    if (!IsValid(CardGameUI))
    {
        FailTask(TEXT("Could not create card game UI widget."));
        return;
    }

    CardGameUI->AddToViewport(0);
    SetupContext->CardGameUI = CardGameUI;

    UCardGameSpawnBucketsTask* const SpawnBucketsTask = NewObject<UCardGameSpawnBucketsTask>(this);
    SpawnBucketsTask->Initialize(SetupContext);
    SpawnBucketsTask->Activate();

    if (!SpawnBucketsTask->WasSuccessful())
    {
        FailTask(SpawnBucketsTask->GetFailureReason());
        return;
    }

    SetupContext->bSwapApplied = true;
    bWasSuccessful = true;
    EndTask();
}

void UCardGameApplySwapSetupTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
