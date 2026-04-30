#include "RiotStoryGameState.h"

#include "Game/RiotStoryGameplayTags.h"
#include "RiotStory.h"
#include "Variant_Shooter/Tasks/CardGameBuildSetupContextTask.h"
#include "Variant_Shooter/Tasks/CardGameEndingGameTask.h"
#include "Variant_Shooter/Tasks/CardGameWaitTransitionEventsTask.h"
#include "Variant_Shooter/Tasks/CardGameWaitTransitionOutEventsTask.h"
#include "UI/CardGameUI.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

void ARiotStoryGameState::BeginPlay()
{
    Super::BeginPlay();
    RegisterEventListeners();
}

void ARiotStoryGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ResetScoreCombo();
    CleanupActiveSetupWaitTask();
    UnregisterEventListeners();
    Super::EndPlay(EndPlayReason);
}

void ARiotStoryGameState::RegisterEventListeners()
{
    CardGameStateListenerHandler = UGameplayMessageSubsystem::Get(this).RegisterListener(
        RiotStoryGameplayTags::TAG_GameCommand_CardThrowGame,
        this,
        &ARiotStoryGameState::HandleCardThrowGameCommand
    );
}

void ARiotStoryGameState::UnregisterEventListeners()
{
    UGameplayMessageSubsystem::Get(this).UnregisterListener(CardGameStateListenerHandler);
    UGameplayMessageSubsystem::Get(this).UnregisterListener(CardThrownListenerHandler);
    UGameplayMessageSubsystem::Get(this).UnregisterListener(CardScoredListenerHandler);
}

void ARiotStoryGameState::PrepareCardThrowingGame()
{
    if (bCardGameSetupInProgress || bCardGameActive)
    {
        UE_LOG(LogRiotStory, Warning, TEXT("Card game start request ignored because setup is in progress or game is already active."));
        return;
    }

    PointsScored = 0;
    NumCardsThrown = 0;
    ResetScoreCombo();
    bCardGameSetupInProgress = true;

    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        RiotStoryGameplayTags::TAG_GameEvent_CardThrowGame_StateChanged,
        FGameEventCardThrowStateGameMessage(EGameEventCardThrowGameState::Starting, EGameEventCardThrowGameResult::None)
    );

    if (!RunBuildSetupContextTask())
    {
        return;
    }

    ActiveSetupWaitTask = NewObject<UCardGameWaitTransitionEventsTask>(this);
    ActiveSetupWaitTask->Initialize(this, &ActiveSetupContext);
    ActiveSetupWaitTask->OnSucceeded.AddUObject(this, &ARiotStoryGameState::HandleSetupTaskFinished);
    ActiveSetupWaitTask->OnFailed.AddUObject(this, &ARiotStoryGameState::HandleSetupTransitionFailed);
    ActiveSetupWaitTask->Activate();
}

void ARiotStoryGameState::StartCardThrowingGame()
{
    UGameplayMessageSubsystem::Get(this).UnregisterListener(CardThrownListenerHandler);
    UGameplayMessageSubsystem::Get(this).UnregisterListener(CardScoredListenerHandler);

    CardThrownListenerHandler = UGameplayMessageSubsystem::Get(this).RegisterListener(
        RiotStoryGameplayTags::TAG_GameEvent_CardThrowGame_CardThrownRemoved,
        this,
        &ARiotStoryGameState::HandleCardThrown
    );

    CardScoredListenerHandler = UGameplayMessageSubsystem::Get(this).RegisterListener(
        RiotStoryGameplayTags::TAG_GameEvent_CardThrowGame_CardScored,
        this,
        &ARiotStoryGameState::HandleCardScored
    );

    bCardGameActive = true;
}

void ARiotStoryGameState::EndCardThrowingGame()
{
    if (!bCardGameActive) return;

    const EGameEventCardThrowGameResult EndResult = PointsScored >= TargetCardGameScore
        ? EGameEventCardThrowGameResult::Succeeded
        : EGameEventCardThrowGameResult::Failed;

    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        RiotStoryGameplayTags::TAG_GameEvent_CardThrowGame_StateChanged,
        FGameEventCardThrowStateGameMessage(EGameEventCardThrowGameState::Ended, EndResult)
    );

    ActiveSetupWaitTask = NewObject<UCardGameWaitTransitionOutEventsTask>(this);
    ActiveSetupWaitTask->Initialize(this, &ActiveSetupContext);
    //TODO: Failed event
    ActiveSetupWaitTask->Activate();

    UGameplayMessageSubsystem::Get(this).UnregisterListener(CardThrownListenerHandler);
    UGameplayMessageSubsystem::Get(this).UnregisterListener(CardScoredListenerHandler);
    bCardGameActive = false;
    ResetScoreCombo();
}

void ARiotStoryGameState::FailCardThrowingGameSetup(const FString& Reason)
{
    UE_LOG(LogRiotStory, Warning, TEXT("Card game setup failed: %s"), *Reason);

    CleanupActiveSetupWaitTask();
    bCardGameSetupInProgress = false;
    bCardGameActive = false;
    ResetScoreCombo();

    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        RiotStoryGameplayTags::TAG_GameEvent_CardThrowGame_StateChanged,
        FGameEventCardThrowStateGameMessage(EGameEventCardThrowGameState::Ended, EGameEventCardThrowGameResult::Failed)
    );
}

void ARiotStoryGameState::CleanupActiveSetupWaitTask()
{
    if (!IsValid(ActiveSetupWaitTask))
    {
        return;
    }

    ActiveSetupWaitTask->OnFailed.RemoveAll(this);
    ActiveSetupWaitTask->StopListening();
    ActiveSetupWaitTask = nullptr;
}

bool ARiotStoryGameState::RunBuildSetupContextTask()
{
    UCardGameBuildSetupContextTask* const BuildContextTask = NewObject<UCardGameBuildSetupContextTask>(this);
    BuildContextTask->Initialize(
        this,
        &ActiveSetupContext,
        StartingCardCount,
        ECustomGameMode::CardGame,
        ECustomGameMode::Default
    );
    BuildContextTask->Activate();

    if (!BuildContextTask->WasSuccessful())
    {
        FailCardThrowingGameSetup(BuildContextTask->GetFailureReason());
        return false;
    }

    return true;
}

void ARiotStoryGameState::HandleSetupTaskFinished()
{
    StartCardThrowingGame();
    bCardGameSetupInProgress = false;
    CleanupActiveSetupWaitTask();
}

void ARiotStoryGameState::HandleSetupTransitionFailed(const FString& Reason)
{
    if (!bCardGameSetupInProgress)
    {
        return;
    }

    FailCardThrowingGameSetup(Reason);
}

void ARiotStoryGameState::HandleCardThrowGameCommand(FGameplayTag Channel, const FGameCommandCardThrowGameMessage& Message)
{
    switch (Message.Type)
    {
        case EGameCommandCardThrowGame::Start:
            PrepareCardThrowingGame();
            break;
        case EGameCommandCardThrowGame::End: //Allows for ending game earlier than normal
            EndCardThrowingGame();
            break;
    }
}

void ARiotStoryGameState::HandleCardThrown(FGameplayTag Channel, const FGameEventCardThrownMessage& Message)
{
    ++NumCardsThrown;

    if (NumCardsThrown >= StartingCardCount)
    {
        const EGameEventCardThrowGameResult EndResult = PointsScored >= TargetCardGameScore
            ? EGameEventCardThrowGameResult::Succeeded
            : EGameEventCardThrowGameResult::Failed;

        //Change state to ending and wait for final end command from player confirmation
        UGameplayMessageSubsystem::Get(this).BroadcastMessage(
            RiotStoryGameplayTags::TAG_GameEvent_CardThrowGame_StateChanged,
            FGameEventCardThrowStateGameMessage(EGameEventCardThrowGameState::Ending, EndResult)
        );

        UCardGameEndingGameTask* const EndingTask = NewObject<UCardGameEndingGameTask>(this);
        EndingTask->Initialize(&ActiveSetupContext, EndResult, PointsScored);
        EndingTask->Activate();
    }
}

void ARiotStoryGameState::HandleCardScored(FGameplayTag Channel, const FGameEventCardScoredMessage& Message)
{
    PointsScored += Message.Points;
    ActiveSetupContext.CardGameUI->BP_UpdateScore(PointsScored);
    ++CurrentScoreCombo;
    RestartScoreComboTimer();
    PlayScoreComboSound();
    DoComboScreenShake();

    //Resetting combo if we have reach the max
    if (CurrentScoreCombo >= MaxCombo)
    {
        ResetScoreCombo();
    }
}

void ARiotStoryGameState::ResetScoreCombo()
{
    CurrentScoreCombo = 0;

    if (UWorld* const World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ScoreComboTimerHandle);
    }
}

void ARiotStoryGameState::RestartScoreComboTimer()
{
    if (ScoreComboDuration <= 0.0f)
    {
        return;
    }

    if (UWorld* const World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ScoreComboTimerHandle,
            this,
            &ARiotStoryGameState::ResetScoreCombo,
            ScoreComboDuration,
            false
        );
    }
}

void ARiotStoryGameState::PlayScoreComboSound() const
{
    USoundBase* SoundToPlay = nullptr;

    if (ScoreComboSounds.Num() > 0)
    {
        const int32 SoundIndex = FMath::Clamp(CurrentScoreCombo - 1, 0, ScoreComboSounds.Num() - 1);
        SoundToPlay = ScoreComboSounds[SoundIndex];
    }

    if (!IsValid(SoundToPlay))
    {
        SoundToPlay = DefaultScoreSound;
    }

    if (IsValid(SoundToPlay))
    {
        UGameplayStatics::PlaySound2D(this, SoundToPlay);
    }

    //Play the punchy sound along with it

    SoundToPlay = nullptr;
    if (ScoreComboPunchSounds.Num() > 0)
    {
        const int32 SoundIndex = FMath::Clamp(CurrentScoreCombo - 1, 0, ScoreComboPunchSounds.Num() - 1);
        SoundToPlay = ScoreComboPunchSounds[SoundIndex];
    }

    if (IsValid(SoundToPlay))
    {
        UGameplayStatics::PlaySound2D(this, SoundToPlay);
    }
}

void ARiotStoryGameState::DoComboScreenShake()
{
    APlayerController *PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        PlayerController->ClientStartCameraShake(CameraShakeClass, CurrentScoreCombo * 0.5);
    }
}
