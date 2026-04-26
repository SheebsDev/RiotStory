#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "Variant_Shooter/Tasks/CardGameSetupTaskContext.h"
#include "CardGameSpawnBucketsTask.generated.h"

UCLASS()
class RIOTSTORY_API UCardGameSpawnBucketsTask : public UGameplayTask
{
    GENERATED_BODY()

public:
    void Initialize(FCardGameSetupTaskContext* InOutContext);

    virtual void Activate() override;

    bool WasSuccessful() const { return bWasSuccessful; }
    const FString& GetFailureReason() const { return FailureReason; }

private:
    FCardGameSetupTaskContext* SetupContext = nullptr;
    bool bWasSuccessful = false;
    FString FailureReason;

    void FailTask(const FString& Reason);
};

