#include "CardGameEndSwapTask.h"

#include "UI/CardGameUI.h"
#include "Variant_Shooter/CardGamePlayerStartPoint.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "Input/GameplayInputTypeControllerInterface.h"

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

    AShooterPlayerController* const ShooterController = SetupContext->ShooterController.Get();
    AShooterCharacter* const ShooterPawn = SetupContext->ShooterPawn.Get();

    const FTransform& ReturnTransform = SetupContext->PlayerReturnTransform;
    ShooterPawn->SetActorLocationAndRotation(ReturnTransform.GetLocation(), ReturnTransform.Rotator());

    SetupContext->CardGameUI->RemoveFromParent();

    bWasSuccessful = true;
    EndTask();
}

void UCardGameEndSwapTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
