#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RiotStoryCommandMessages.h"
#include "ModeTransitionUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionSwapPointDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionEndedDelegate);

/**
 *  Transition Mode animation widget
 */
UCLASS(abstract)
class RIOTSTORY_API UModeTransitionUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Update the widget with the interaction meta data (Better to use TriggerStartTransition for OnTransitionStarted delegate to be triggered) */
	UFUNCTION(BlueprintImplementableEvent, Category="ModeTransition", meta=(DisplayName = "StartTransition"))
    void BP_StartTransition();

    /** Manually trigger the start transition with this method */
    UFUNCTION(BlueprintCallable, Category="ModeTransition")
    void TriggerStartTransition();

    UFUNCTION(BlueprintCallable, Category="ModeTransition")
    void SetTransitionModes(ECustomGameMode InNewMode, ECustomGameMode InCurrentMode);

protected:

    UFUNCTION(BlueprintCallable, Category="ModeTransition")
    void NotifyTransitionSwapPoint();

    UFUNCTION(BlueprintCallable, Category="ModeTransition")
    void NotifyTransitionEnded();

public:
    UPROPERTY(BlueprintAssignable, Category="ModeTransition")
    FOnTransitionStartedDelegate OnTransitionStarted;

    UPROPERTY(BlueprintAssignable, Category="ModeTransition")
    FOnTransitionSwapPointDelegate OnTransitionSwapPoint;

    UPROPERTY(BlueprintAssignable, Category="ModeTransition")
    FOnTransitionEndedDelegate OnTransitionEnded;

private:
    ECustomGameMode TransitionNewMode = ECustomGameMode::Default;
    ECustomGameMode TransitionCurrentMode = ECustomGameMode::Default;
};
