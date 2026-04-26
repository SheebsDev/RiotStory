#include "Variant_Shooter/Tasks/CardGameBuildSetupContextTask.h"

#include "Kismet/GameplayStatics.h"
#include "UI/ModeTransitionUI.h"
#include "Variant_Shooter/CardGamePlayerStartPoint.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"

void UCardGameBuildSetupContextTask::Initialize(
    UObject* InWorldContextObject,
    FCardGameSetupTaskContext* InOutContext,
    const int32 InStartingCardCount,
    const ECustomGameMode InNewMode,
    const ECustomGameMode InCurrentMode)
{
    WorldContextObject = InWorldContextObject;
    SetupContext = InOutContext;
    StartingCardCount = InStartingCardCount;
    NewMode = InNewMode;
    CurrentMode = InCurrentMode;
}

void UCardGameBuildSetupContextTask::Activate()
{
    bWasSuccessful = false;
    FailureReason.Reset();

    if (!WorldContextObject.IsValid())
    {
        FailTask(TEXT("World context is invalid."));
        return;
    }

    UWorld* const World = WorldContextObject->GetWorld();
    if (!IsValid(World))
    {
        FailTask(TEXT("Could not resolve world from context object."));
        return;
    }

    if (!SetupContext)
    {
        FailTask(TEXT("Setup context pointer is null."));
        return;
    }

    AShooterPlayerController* const ShooterController = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (!IsValid(ShooterController))
    {
        FailTask(TEXT("Could not resolve shooter player controller."));
        return;
    }

    AShooterCharacter* const ShooterPawn = ShooterController->GetShooterPawn();
    if (!IsValid(ShooterPawn))
    {
        FailTask(TEXT("Could not resolve shooter pawn."));
        return;
    }

    ACardGamePlayerStartPoint* const StartPoint = Cast<ACardGamePlayerStartPoint>(UGameplayStatics::GetActorOfClass(World, ACardGamePlayerStartPoint::StaticClass()));
    if (!IsValid(StartPoint))
    {
        FailTask(TEXT("Could not find card game player start point actor."));
        return;
    }

    UModeTransitionUI* const TransitionUI = ShooterController->GetOrCreateModeTransitionUI();
    if (!IsValid(TransitionUI))
    {
        FailTask(TEXT("Could not create mode transition UI."));
        return;
    }

    SetupContext->ShooterController = ShooterController;
    SetupContext->ShooterPawn = ShooterPawn;
    SetupContext->CardGameStartPoint = StartPoint;
    SetupContext->ModeTransitionUI = TransitionUI;
    SetupContext->StartingCardCount = FMath::Max(0, StartingCardCount);
    SetupContext->NewMode = NewMode;
    SetupContext->CurrentMode = CurrentMode;
    SetupContext->bSwapApplied = false;

    bWasSuccessful = true;
    EndTask();
}

void UCardGameBuildSetupContextTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
