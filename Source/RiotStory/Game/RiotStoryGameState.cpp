#include "RiotStoryGameState.h"

#include "Game/RiotStoryGameplayTags.h"
#include "RiotStory.h"
#include "Variant_Shooter/Tasks/CardGameBuildSetupContextTask.h"
#include "Variant_Shooter/Tasks/CardGameWaitTransitionEventsTask.h"
#include "Variant_Shooter/Tasks/CardGameWaitTransitionOutEventsTask.h"

void ARiotStoryGameState::BeginPlay()
{
    Super::BeginPlay();
    RegisterEventListeners();
}

void ARiotStoryGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
        RiotStoryGameplayTags::TAG_GameEvent_CardThrowGame_CardThrown,
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
}

void ARiotStoryGameState::FailCardThrowingGameSetup(const FString& Reason)
{
    UE_LOG(LogRiotStory, Warning, TEXT("Card game setup failed: %s"), *Reason);

    CleanupActiveSetupWaitTask();
    bCardGameSetupInProgress = false;
    bCardGameActive = false;

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
        EndCardThrowingGame();
    }
}

void ARiotStoryGameState::HandleCardScored(FGameplayTag Channel, const FGameEventCardScoredMessage& Message)
{
    PointsScored += Message.Points;
}
