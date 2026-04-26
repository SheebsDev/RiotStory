#include "CardGameWaitTransitionOutEventsTask.h"
#include "CardGameEndSwapTask.h"
#include "CardGameFinalizeEndTask.h"

void UCardGameWaitTransitionOutEventsTask::HandleTransitionStarted(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message)
{
    //Empty currently
}

void UCardGameWaitTransitionOutEventsTask::HandleTransitionSwap(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message)
{
    UCardGameEndSwapTask* const GameEndSwapTask = NewObject<UCardGameEndSwapTask>(this);
    GameEndSwapTask->Initialize(SetupContext);
    GameEndSwapTask->Activate();
}

void UCardGameWaitTransitionOutEventsTask::HandleTransitionEnded(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message)
{
    UCardGameFinalizeEndTask* const FinalizeEndTask = NewObject<UCardGameFinalizeEndTask>(this);
    FinalizeEndTask->Initialize(SetupContext);
    FinalizeEndTask->Activate();
    
    CleanupListeners();
    EndTask();
}