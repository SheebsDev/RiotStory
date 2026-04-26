#include "Variant_Shooter/Tasks/CardGameApplySwapSetupTask.h"

#include "UI/CardGameUI.h"
#include "Variant_Shooter/CardGamePlayerStartPoint.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Input/GameplayInputTypeControllerInterface.h"

void UCardGameApplySwapSetupTask::Initialize(FCardGameSetupTaskContext* InOutContext)
{
    SetupContext = InOutContext;
}

void UCardGameApplySwapSetupTask::Activate()
{
    bWasSuccessful = false;
    FailureReason.Reset();

    if (!SetupContext)
    {
        FailTask(TEXT("Setup context is null."));
        return;
    }

    if (!SetupContext->IsValidForSwap())
    {
        FailTask(TEXT("Setup context is missing required swap references."));
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
    SetupContext->bSwapApplied = true;
    bWasSuccessful = true;
    EndTask();
}

void UCardGameApplySwapSetupTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
