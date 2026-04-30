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
class USoundBase;
class UCameraShakeBase;

UCLASS(Blueprintable)
class RIOTSTORY_API ARiotStoryGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintPure, Category="Cards|Combo")
    int32 GetCurrentScoreCombo() const { return CurrentScoreCombo; }

    /** The starting number of cards for the card throwing game */
    UPROPERTY(EditAnywhere, Category="Cards")
    int32 StartingCardCount = 10;

    /** The baseline success condition for the card throwing game */
    UPROPERTY(EditAnywhere, Category="Cards")
    int32 TargetCardGameScore = 3;

    /** Time window in seconds to continue a score combo before it expires. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cards|Combo", meta=(ClampMin="0.0"))
    float ScoreComboDuration = 10.0f;

    /** the maximum amount of score combo allowed before it resets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cards|Combo")
    int MaxCombo = 5;

    /** Optional combo-indexed score sounds. Index 0 is combo 1, index 1 is combo 2, etc. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cards|Combo|Audio")
    TArray<TObjectPtr<USoundBase>> ScoreComboSounds;

    /** Optional combo-indexed score sounds. Index 0 is combo 1, index 1 is combo 2, etc. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cards|Combo|Audio")
    TArray<TObjectPtr<USoundBase>> ScoreComboPunchSounds;

    /** Optional fallback sound when there is no sound set for the current combo index. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cards|Combo|Audio")
    TObjectPtr<USoundBase> DefaultScoreSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cards|Combo|Camera")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

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
    int32 CurrentScoreCombo = 0;
    bool bCardGameSetupInProgress = false;
    bool bCardGameActive = false;
    FTimerHandle ScoreComboTimerHandle;

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
    
    void ResetScoreCombo();
    void RestartScoreComboTimer();
    void PlayScoreComboSound() const;

    void DoComboScreenShake();
};
