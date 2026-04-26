#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Conversation/ConversationMessages.h"
#include "ConversationUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConversationResponseIndexChosenDelegate, int32, ResponseIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FConversationWindowClickedDelegate);

/**
 *  UI for engaging in conversations with NPCs/Other
 */
UCLASS(abstract)
class RIOTSTORY_API UConversationUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Update the widget with the latest conversation text */
	UFUNCTION(BlueprintImplementableEvent, Category="Conversation", meta=(DisplayName = "UpdateConversationText"))
    void BP_UpdateConversationText(const FText& Text, bool bIsLast);

    /** Show the conversation window with the text and set the state if it is the last chunk of text */
    UFUNCTION(BlueprintImplementableEvent, Category="Conversation", meta=(DisplayName = "ShowConversation"))
    void BP_ShowConversation(const FText& Text, bool bIsLast);

    /** Display the response window with the listed responses */
    UFUNCTION(BlueprintImplementableEvent, Category="Conversation", meta=(DisplayName = "ShowResponses"))
    void BP_ShowResponses(const TArray<FConversationResponseOptionDisplay>& Responses);

	/** Response event for outside or external events to hook into the UI */
	UFUNCTION(BlueprintImplementableEvent, Category="Conversation", meta=(DisplayName = "ClearOutResponses"))
	void BP_ClearOutResponses();

public:
	/** Fired when the player selects a response option index from the conversation UI. */
	UPROPERTY(BlueprintAssignable, Category="Conversation")
	FConversationResponseIndexChosenDelegate OnResponseIndexChosen;

	/** Fired when the conversation window is clicked/selected to continue. */
	UPROPERTY(BlueprintAssignable, Category="Conversation")
	FConversationWindowClickedDelegate OnConversationWindowClicked;

	/** Broadcast the selected response index to any delegate listeners. */
	UFUNCTION(BlueprintCallable, Category="Conversation")
	void NotifyResponseIndexChosen(int32 ResponseIndex);

	/** Broadcast that the conversation window has been clicked. */
	UFUNCTION(BlueprintCallable, Category="Conversation", meta=(DisplayName = "NotifyConversationWindowSelected"))
	void NotifyConversationWindowClicked();
};
