#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TicketUI.generated.h"

/**
 *  Simple bullet counter UI widget for a first person shooter game
 */
UCLASS(abstract)
class RIOTSTORY_API UTicketUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Update the widget with the interaction meta data */
	UFUNCTION(BlueprintImplementableEvent, Category="Ticket", meta=(DisplayName = "AddTicket"))
    void BP_AddTicket();

    UFUNCTION(BlueprintImplementableEvent, Category="Ticket", meta=(DisplayName = "RemoveTicket"))
    void BP_RemoveTicket();
};