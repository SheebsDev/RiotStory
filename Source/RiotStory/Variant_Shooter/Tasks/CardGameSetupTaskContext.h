#pragma once

#include "CoreMinimal.h"
#include "RiotStoryCommandMessages.h"
#include "CardGameSetupTaskContext.generated.h"

class ACardGamePlayerStartPoint;
class AShooterCharacter;
class AShooterPlayerController;
class UCardGameUI;
class UModeTransitionUI;

USTRUCT()
struct RIOTSTORY_API FCardGameSetupTaskContext
{
    GENERATED_BODY()

    TWeakObjectPtr<AShooterPlayerController> ShooterController = nullptr;
    TWeakObjectPtr<AShooterCharacter> ShooterPawn = nullptr;
    TWeakObjectPtr<ACardGamePlayerStartPoint> CardGameStartPoint = nullptr;
    TWeakObjectPtr<UModeTransitionUI> ModeTransitionUI = nullptr;
    TWeakObjectPtr<UCardGameUI> CardGameUI = nullptr;

    FTransform PlayerReturnTransform = FTransform::Identity;

    int32 StartingCardCount = 0;
    ECustomGameMode NewMode = ECustomGameMode::Default;
    ECustomGameMode CurrentMode = ECustomGameMode::Default;

    bool bSwapApplied = false;

    bool IsValidForTransition() const;
    bool IsValidForSwap() const;
    bool IsValidForFinalize() const;
};
