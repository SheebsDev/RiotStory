#include "Variant_Shooter/Tasks/CardGameEndingGameTask.h"

#include "UI/CardGameUI.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"

void UCardGameEndingGameTask::Initialize(
    FCardGameSetupTaskContext* InOutContext,
    EGameEventCardThrowGameResult InResult,
    int32 InFinalScore)
{
    SetupContext = InOutContext;
    EndResult = InResult;
    FinalScore = InFinalScore;
}

void UCardGameEndingGameTask::Activate()
{
    bWasSuccessful = false;
    FailureReason.Reset();

    if (!SetupContext || !SetupContext->IsValidForFinalize())
    {
        FailTask(TEXT("Setup context is null or has invalid data."));
        return;
    }

    AShooterPlayerController* const ShooterController = SetupContext->ShooterController.Get();
    AShooterCharacter* const ShooterPawn = SetupContext->ShooterPawn.Get();
    UCardGameUI* const CardGameUI = SetupContext->CardGameUI.Get();

    if (!IsValid(ShooterController) || !IsValid(ShooterPawn) || !IsValid(CardGameUI))
    {
        FailTask(TEXT("Ending task requires valid shooter controller, pawn, and card game UI."));
        return;
    }

    ShooterController->SetAllGameplayInputTypesEnabled(false);
    ShooterController->SetInputMode(FInputModeUIOnly());
    ShooterController->SetShowMouseCursor(true);
    ShooterController->bEnableClickEvents = true;
    ShooterController->bEnableMouseOverEvents = true;

    CardGameUI->BP_ShowEndScreen(EndResult, FinalScore);

    bWasSuccessful = true;
    EndTask();
}

void UCardGameEndingGameTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
