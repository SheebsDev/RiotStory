#include "ModeTransitionUI.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "RiotStoryEventMessages.h"
#include "Game/RiotStoryGameplayTags.h"

void UModeTransitionUI::SetTransitionModes(const ECustomGameMode InNewMode, const ECustomGameMode InCurrentMode)
{
    TransitionNewMode = InNewMode;
    TransitionCurrentMode = InCurrentMode;
}

void UModeTransitionUI::NotifyTransitionSwapPoint()
{
    OnTransitionSwapPoint.Broadcast();
    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        RiotStoryGameplayTags::TAG_GameEvent_ModeTransitionAtSwap,
        FGameEventModeTransitionMessage(TransitionNewMode, TransitionCurrentMode)
    );
}

void UModeTransitionUI::NotifyTransitionEnded()
{
    OnTransitionEnded.Broadcast();
    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        RiotStoryGameplayTags::TAG_GameEvent_ModeTransitionEnded,
        FGameEventModeTransitionMessage(TransitionNewMode, TransitionCurrentMode)
    );
}

void UModeTransitionUI::TriggerStartTransition()
{
    OnTransitionStarted.Broadcast();
    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        RiotStoryGameplayTags::TAG_GameEvent_ModeTransitionStarted,
        FGameEventModeTransitionMessage(TransitionNewMode, TransitionCurrentMode)  
    );

    //NOTE:Generally not synchronous, but if it is other items keying off of events may have problems
    //so we are calling this after broadcasting the started event
    BP_StartTransition(); 
}
