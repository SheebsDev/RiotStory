#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "RiotStoryEventMessages.h"
#include "RiotStoryCommandMessages.h"
#include "Variant_Shooter/Tasks/CardGameSetupTaskContext.h"
#include "RiotStoryGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCardThrowGameStateChangedDelegate, EGameEventCardThrowGameState, State);

class UCardGameWaitTransitionEventsTask;

UCLASS(Blueprintable)
class RIOTSTORY_API ARiotStoryGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** The starting number of cards for the card throwing game */
    UPROPERTY(EditAnywhere, Category="Cards")
    int32 StartingCardCount = 10;

    /** The baseline success condition for the card throwing game */
    UPROPERTY(EditAnywhere, Category="Cards")
    int32 TargetCardGameScore = 3;

protected:


private:
    FGameplayMessageListenerHandle CardGameStateListenerHandler;
    FGameplayMessageListenerHandle CardThrownListenerHandler;
    FGameplayMessageListenerHandle CardScoredListenerHandler;

    UPROPERTY()
    TObjectPtr<UCardGameWaitTransitionEventsTask> ActiveSetupWaitTask;

    FCardGameSetupTaskContext ActiveSetupContext;

    int32 NumCardsThrown = 0;
    int32 PointsScored = 0;
    bool bCardGameSetupInProgress = false;
    bool bCardGameActive = false;

private:
    void RegisterEventListeners();
    void UnregisterEventListeners();

    void PrepareCardThrowingGame();
    void StartCardThrowingGame();
    void EndCardThrowingGame();
    void FailCardThrowingGameSetup(const FString& Reason);
    void CleanupActiveSetupWaitTask();

    bool RunBuildSetupContextTask();
    void HandleSetupTaskFinished();
    void HandleSetupTransitionFailed(const FString& Reason);

    void HandleCardThrowGameCommand(FGameplayTag Channel, const FGameCommandCardThrowGameMessage& Message);
    void HandleCardThrown(FGameplayTag Channel, const FGameEventCardThrownMessage& Message);
    void HandleCardScored(FGameplayTag Channel, const FGameEventCardScoredMessage& Message);
};
