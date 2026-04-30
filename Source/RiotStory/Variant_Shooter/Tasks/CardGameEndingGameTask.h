#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "RiotStoryEventMessages.h"
#include "Variant_Shooter/Tasks/CardGameSetupTaskContext.h"
#include "CardGameEndingGameTask.generated.h"

UCLASS()
class RIOTSTORY_API UCardGameEndingGameTask : public UGameplayTask
{
    GENERATED_BODY()

public:
    void Initialize(
        FCardGameSetupTaskContext* InOutContext,
        EGameEventCardThrowGameResult InResult,
        int32 InFinalScore);

    virtual void Activate() override;

    bool WasSuccessful() const { return bWasSuccessful; }
    const FString& GetFailureReason() const { return FailureReason; }

private:
    FCardGameSetupTaskContext* SetupContext = nullptr;
    EGameEventCardThrowGameResult EndResult = EGameEventCardThrowGameResult::None;
    int32 FinalScore = 0;
    bool bWasSuccessful = false;
    FString FailureReason;

    void FailTask(const FString& Reason);
};
