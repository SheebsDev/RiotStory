#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardGameUI.generated.h"

UCLASS(abstract)
class RIOTSTORY_API UCardGameUI : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Update the current score of the card game */
	UFUNCTION(BlueprintImplementableEvent, Category="CardGame", meta=(DisplayName = "UpdateScore"))
    void BP_UpdateScore(int32 Score);

    /** Update the card counter on the screen for the remaining cards to throw */
    UFUNCTION(BlueprintImplementableEvent, Category="CardGame", meta=(DisplayName = "UpdateCardCount"))
    void BP_UpdateCardCount(int32 Count);
};