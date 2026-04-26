#pragma once

#include "CoreMinimal.h"
#include "RiotStoryCommandMessages.generated.h"

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameCommandMessage
{
    //Empty placeholder as commands don't typically need data
    GENERATED_BODY()
};

//TODO: May want to move this to a better location
UENUM(BlueprintType)
enum class ECustomGameMode : uint8
{
    Default,
    CardGame
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameCommandModeTransitionMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameCommand|Transition")
    ECustomGameMode NewMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameCommand|Transition")
    ECustomGameMode CurrentMode;

    FGameCommandModeTransitionMessage() = default;
    FGameCommandModeTransitionMessage(ECustomGameMode NewMode, ECustomGameMode CurrentMode)
        : NewMode(NewMode), CurrentMode(CurrentMode) { }
};

UENUM(BlueprintType)
enum class EGameCommandCardThrowGame : uint8
{
    Start,
    End
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FGameCommandCardThrowGameMessage
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GameCommand|CardThrow")
    EGameCommandCardThrowGame Type;
};