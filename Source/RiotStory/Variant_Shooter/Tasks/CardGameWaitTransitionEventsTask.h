#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "RiotStoryCommandMessages.h"
#include "CardGameSetupTaskContext.h"
#include "CardGameWaitTransitionEventsTask.generated.h"

struct FGameEventModeTransitionMessage;

DECLARE_MULTICAST_DELEGATE(FCardGameTransitionSucceededDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FCardGameTransitionFailedDelegate, const FString&);

UCLASS()
class RIOTSTORY_API UCardGameWaitTransitionEventsTask : public UGameplayTask
{
    GENERATED_BODY()

public:
    void Initialize(UObject* InWorldContextObject, FCardGameSetupTaskContext* InSetupContext);

    virtual void Activate() override;

    void StopListening();

public:
    FCardGameTransitionSucceededDelegate OnSucceeded;
    FCardGameTransitionFailedDelegate OnFailed;

protected:
    FCardGameSetupTaskContext* SetupContext;

    virtual void HandleTransitionStarted(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message);
    virtual void HandleTransitionSwap(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message);
    virtual void HandleTransitionEnded(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message);
    void CleanupListeners();

private:
    TWeakObjectPtr<UObject> WorldContextObject = nullptr;

    FGameplayMessageListenerHandle TransitionStartedListener;
    FGameplayMessageListenerHandle TransitionSwapListener;
    FGameplayMessageListenerHandle TransitionEndedListener;

    bool RunApplySwapSetupTask();
    bool RunFinalizeStartTask();
};
