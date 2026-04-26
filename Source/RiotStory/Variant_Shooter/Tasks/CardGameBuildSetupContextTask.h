#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "Variant_Shooter/Tasks/CardGameSetupTaskContext.h"
#include "CardGameBuildSetupContextTask.generated.h"

UCLASS()
class RIOTSTORY_API UCardGameBuildSetupContextTask : public UGameplayTask
{
    GENERATED_BODY()

public:
    void Initialize(UObject* InWorldContextObject, FCardGameSetupTaskContext* InOutContext, int32 InStartingCardCount, ECustomGameMode InNewMode, ECustomGameMode InCurrentMode);

    virtual void Activate() override;

    bool WasSuccessful() const { return bWasSuccessful; }
    const FString& GetFailureReason() const { return FailureReason; }

private:
    TWeakObjectPtr<UObject> WorldContextObject = nullptr;
    FCardGameSetupTaskContext* SetupContext = nullptr;
    int32 StartingCardCount = 0;
    ECustomGameMode NewMode = ECustomGameMode::Default;
    ECustomGameMode CurrentMode = ECustomGameMode::Default;

    bool bWasSuccessful = false;
    FString FailureReason;

    void FailTask(const FString& Reason);
};
