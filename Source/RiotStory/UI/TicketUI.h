#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TicketUI.generated.h"

/**
 *  Simple Ticket UI HUD
 */
UCLASS(abstract)
class RIOTSTORY_API UTicketUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, Category="Ticket", meta=(DisplayName = "AddTicket"))
    void BP_AddTicket();

    UFUNCTION(BlueprintImplementableEvent, Category="Ticket", meta=(DisplayName = "RemoveTicket"))
    void BP_RemoveTicket();
};