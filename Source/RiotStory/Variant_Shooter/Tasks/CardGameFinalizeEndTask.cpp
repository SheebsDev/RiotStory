#include "CardGameFinalizeEndTask.h"

#include "Input/GameplayInputTypeControllerInterface.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"

void UCardGameFinalizeEndTask::Initialize(FCardGameSetupTaskContext* InOutContext)
{
    SetupContext = InOutContext;
}

void UCardGameFinalizeEndTask::Activate()
{
    bWasSuccessful = false;
    FailureReason.Reset();

    if (!SetupContext || !SetupContext->IsValidForFinalize())
    {
        FailTask(TEXT("Setup context is null or has invalid data"));
        return;
    }

    AShooterPlayerController* ShooterController = SetupContext->ShooterController.Get();
    AShooterCharacter* const ShooterPawn = SetupContext->ShooterPawn.Get();

    ShooterController->SetInputMode(FInputModeGameOnly());
    ShooterController->SetShowMouseCursor(false);
    ShooterController->bEnableClickEvents = false;
    ShooterController->bEnableMouseOverEvents = false;

    //Reset base input type
    ShooterController->SetCurrentModeInputTypes(EGameplayInputType::All);
    ShooterPawn->SetEnableCardThrowing(false);

    bWasSuccessful = true;
    EndTask();
}

void UCardGameFinalizeEndTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
