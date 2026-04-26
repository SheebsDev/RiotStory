#include "CardGameEndSwapTask.h"

#include "UI/CardGameUI.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/Tasks/CardGameDespawnBucketsTask.h"

void UCardGameEndSwapTask::Initialize(FCardGameSetupTaskContext* InOutContext)
{
    SetupContext = InOutContext;
}

void UCardGameEndSwapTask::Activate()
{
    bWasSuccessful = false;
    FailureReason.Reset();

    if (!SetupContext || ! SetupContext->IsValidForSwap())
    {
        FailTask(TEXT("Setup context is null or invalid."));
        return;
    }

    AShooterCharacter* const ShooterPawn = SetupContext->ShooterPawn.Get();

    const FTransform& ReturnTransform = SetupContext->PlayerReturnTransform;
    ShooterPawn->SetActorLocationAndRotation(ReturnTransform.GetLocation(), ReturnTransform.Rotator());

    if (UCardGameUI* const CardGameUI = SetupContext->CardGameUI.Get())
    {
        CardGameUI->RemoveFromParent();
    }

    UCardGameDespawnBucketsTask* const DespawnBucketsTask = NewObject<UCardGameDespawnBucketsTask>(this);
    DespawnBucketsTask->Initialize(SetupContext);
    DespawnBucketsTask->Activate();

    if (!DespawnBucketsTask->WasSuccessful())
    {
        FailTask(DespawnBucketsTask->GetFailureReason());
        return;
    }

    bWasSuccessful = true;
    EndTask();
}

void UCardGameEndSwapTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
