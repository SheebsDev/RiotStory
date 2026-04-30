#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RiotStoryEventMessages.h"
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

    /** Update The max number of cards to be displayed */
    UFUNCTION(BlueprintImplementableEvent, Category="CardGame", meta=(DisplayName = "UpdateMaxCardCount"))
    void BP_UpdateMaxCardCount(int32 Count);

    UFUNCTION(BlueprintImplementableEvent, Category="CardGame", meta=(DisplayName = "UpdateChargePercentage"))
    void BP_UpdateChargePercentage(float Percent);

    UFUNCTION(BlueprintImplementableEvent, Category="CardGame", meta=(DisplayName = "ShowEndScreen"))
    void BP_ShowEndScreen(EGameEventCardThrowGameResult Result, int32 FinalScore);

    UFUNCTION(BlueprintImplementableEvent, Category="CardGame", meta=(DisplayName = "ShowStartingScreen"))
    void BP_ShowStartingScreen();

    UFUNCTION(BlueprintCallable, Category="CardGame")
    void NotifyContinueClicked();
};
