#include "CardGameWaitTransitionEventsTask.h"
#include "CardGameApplySwapSetupTask.h"
#include "CardGameFinalizeStartTask.h"
#include "CardGameSetupTaskContext.h"
#include "Game/RiotStoryGameplayTags.h"
#include "RiotStoryEventMessages.h"

void UCardGameWaitTransitionEventsTask::Initialize(
    UObject* InWorldContextObject,
    FCardGameSetupTaskContext* InSetupContext)
{
    WorldContextObject = InWorldContextObject;
    SetupContext = InSetupContext;
}

void UCardGameWaitTransitionEventsTask::Activate()
{
    if (!WorldContextObject.IsValid())
    {
        OnFailed.Broadcast(TEXT("WaitTransitionEventsTask has invalid world context."));
        EndTask();
        return;
    }

    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(WorldContextObject.Get());
    TransitionStartedListener = MessageSubsystem.RegisterListener<FGameEventModeTransitionMessage>(
        RiotStoryGameplayTags::TAG_GameEvent_ModeTransitionStarted,
        this,
        &UCardGameWaitTransitionEventsTask::HandleTransitionStarted
    );

    TransitionSwapListener = MessageSubsystem.RegisterListener<FGameEventModeTransitionMessage>(
        RiotStoryGameplayTags::TAG_GameEvent_ModeTransitionAtSwap,
        this,
        &UCardGameWaitTransitionEventsTask::HandleTransitionSwap
    );

    TransitionEndedListener = MessageSubsystem.RegisterListener<FGameEventModeTransitionMessage>(
        RiotStoryGameplayTags::TAG_GameEvent_ModeTransitionEnded,
        this,
        &UCardGameWaitTransitionEventsTask::HandleTransitionEnded
    );

    MessageSubsystem.BroadcastMessage(
        RiotStoryGameplayTags::TAG_GameCommand_StartModeTransition,
        FGameCommandModeTransitionMessage(SetupContext->NewMode, SetupContext->CurrentMode)
    );
}

void UCardGameWaitTransitionEventsTask::StopListening()
{
    CleanupListeners();
    EndTask();
}

void UCardGameWaitTransitionEventsTask::CleanupListeners()
{
    if (!WorldContextObject.IsValid()) return;

    UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(WorldContextObject.Get());
    if (TransitionStartedListener.IsValid())
    {
        MessageSubsystem.UnregisterListener(TransitionStartedListener);
    }

    if (TransitionSwapListener.IsValid())
    {
        MessageSubsystem.UnregisterListener(TransitionSwapListener);
    }

    if (TransitionEndedListener.IsValid())
    {
        MessageSubsystem.UnregisterListener(TransitionEndedListener);
    }
}

bool UCardGameWaitTransitionEventsTask::RunApplySwapSetupTask()
{
    UCardGameApplySwapSetupTask* const ApplySwapTask = NewObject<UCardGameApplySwapSetupTask>(this);
    ApplySwapTask->Initialize(SetupContext);
    ApplySwapTask->Activate();

    return ApplySwapTask->WasSuccessful();
}

bool UCardGameWaitTransitionEventsTask::RunFinalizeStartTask()
{
    UCardGameFinalizeStartTask* const FinalizeStartTask = NewObject<UCardGameFinalizeStartTask>(this);
    FinalizeStartTask->Initialize(SetupContext);
    FinalizeStartTask->Activate();

    return FinalizeStartTask->WasSuccessful();
}

void UCardGameWaitTransitionEventsTask::HandleTransitionStarted(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message)
{
    //Empty currently
}

void UCardGameWaitTransitionEventsTask::HandleTransitionSwap(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message)
{
    if (!RunApplySwapSetupTask())
    {
        OnFailed.Broadcast(TEXT("Apply Swap Setup Task has failed."));
    }
}

void UCardGameWaitTransitionEventsTask::HandleTransitionEnded(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message)
{
    if (RunFinalizeStartTask())
    {
        OnSucceeded.Broadcast();
    }
    else
    {
        OnFailed.Broadcast(TEXT("Finalize Start Task has failed."));
    }

    CleanupListeners();
    EndTask();
}
