#include "Variant_Shooter/Tasks/CardGameFinalizeStartTask.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Game/RiotStoryGameplayTags.h"
#include "RiotStoryEventMessages.h"
#include "UI/CardGameUI.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "Variant_Shooter/ShooterPlayerController.h"

void UCardGameFinalizeStartTask::Initialize(FCardGameSetupTaskContext* InOutContext)
{
    SetupContext = InOutContext;
}

void UCardGameFinalizeStartTask::Activate()
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

    // During card game setup the player should keep look/combat but lose movement and interact.
    const EGameplayInputType DisabledDuringGame = EGameplayInputType::Move | EGameplayInputType::Interact;
    ShooterController->SetCurrentModeInputTypes(~DisabledDuringGame);
    ShooterPawn->SetEnableCardThrowing(true);
    ShooterPawn->SetAvailableNumberOfCards(SetupContext->StartingCardCount);

    if (UCardGameUI* const CardGameUI = SetupContext->CardGameUI.Get())
    {
        CardGameUI->BP_UpdateScore(0);
        CardGameUI->BP_UpdateCardCount(SetupContext->StartingCardCount);
    }

    UGameplayMessageSubsystem::Get(ShooterController).BroadcastMessage(
        RiotStoryGameplayTags::TAG_GameEvent_CardThrowGame_StateChanged,
        FGameEventCardThrowStateGameMessage(EGameEventCardThrowGameState::Started, EGameEventCardThrowGameResult::None)
    );

    bWasSuccessful = true;
    EndTask();
}

void UCardGameFinalizeStartTask::FailTask(const FString& Reason)
{
    FailureReason = Reason;
    EndTask();
}
