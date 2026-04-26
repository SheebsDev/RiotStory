#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractUI.generated.h"

/**
 *  Interact UI for when encountering an interactable object or NPC
 */
UCLASS(abstract)
class RIOTSTORY_API UInteractUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Update the widget with the interaction meta data */
	UFUNCTION(BlueprintImplementableEvent, Category="Interact", meta=(DisplayName = "UpdateIteract"))
    void BP_UpdateInteractText(const FText& Text);

    /** Toggles the Interact HUD to be visible */
    UFUNCTION(BlueprintImplementableEvent, Category="Interact", meta=(DisplayName = "ShowInteract"))
    void BP_ShowInteract();

    /** Toggles the Interact HUD to not be visible */
    UFUNCTION(BlueprintImplementableEvent, Category="Interact", meta=(DisplayName = "HideInteract"))
    void BP_HideInteract();
};