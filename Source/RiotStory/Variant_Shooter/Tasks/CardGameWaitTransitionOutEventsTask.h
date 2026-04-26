#pragma once

#include "CoreMinimal.h"
#include "CardGameWaitTransitionEventsTask.h"
#include "CardGameWaitTransitionOutEventsTask.generated.h"

UCLASS()
class RIOTSTORY_API UCardGameWaitTransitionOutEventsTask : public UCardGameWaitTransitionEventsTask
{
    GENERATED_BODY()

protected:
    void HandleTransitionStarted(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message) override;
    void HandleTransitionSwap(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message) override;
    void HandleTransitionEnded(FGameplayTag Channel, const FGameEventModeTransitionMessage& Message) override;
};