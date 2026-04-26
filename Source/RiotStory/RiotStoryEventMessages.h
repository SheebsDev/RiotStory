#pragma once

#include "CoreMinimal.h"
#include "RiotStoryCommandMessages.h"
#include "RiotStoryEventMessages.generated.h"

UENUM(BlueprintType)
enum class EGameEventStateType : uint8
{
	Start,
	End,
	Pause
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameEventMessage
{
    GENERATED_BODY()
    //Empty Placeholder for events that have no message data
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameEventStateMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameEvents")
    EGameEventStateType Type;
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameEventModeTransitionMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameEvents|Transition")
    ECustomGameMode NewMode = ECustomGameMode::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameEvents|Transition")
    ECustomGameMode CurrentMode = ECustomGameMode::Default;

    FGameEventModeTransitionMessage() = default;
    FGameEventModeTransitionMessage(const ECustomGameMode InNewMode, const ECustomGameMode InCurrentMode)
        : NewMode(InNewMode), CurrentMode(InCurrentMode)
    {
    }
};

UENUM(BlueprintType)
enum class EGameEventCardThrowGameState : uint8
{
    Starting,
    Started,
    InProgress,
    Ending,
    Ended
};

UENUM(BlueprintType)
enum class EGameEventCardThrowGameResult : uint8
{
    None,
    Succeeded,
    Failed
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameEventCardThrowStateGameMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameEvents|CardThrow")
    EGameEventCardThrowGameState Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameEvents|CardThrow")
    EGameEventCardThrowGameResult Result;

    FGameEventCardThrowStateGameMessage() = default;
    FGameEventCardThrowStateGameMessage(const EGameEventCardThrowGameState Type, const EGameEventCardThrowGameResult Result) : Type(Type), Result(Result) {}
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameEventCardThrownMessage
{
    GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameEventCardScoredMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameEvents|CardThrow")
    int32 Points;
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameEventDrinksMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameEvents|Drinks")
    int32 ChosenDrinkId = -1;
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameEventStoryMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameEvents|Story")
    int32 WatchedId = -1;
};
