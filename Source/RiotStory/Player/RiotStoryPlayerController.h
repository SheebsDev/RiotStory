// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Conversation/ConversationMessages.h"
#include "Inventory/InventoryComponent.h"
#include "Interaction/InteracteeTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Input/GameplayInputTypeControllerInterface.h"
#include "RiotStoryEventMessages.h"
#include "RiotStoryCommandMessages.h"
#include "RiotStoryPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UInteractUI;
class UInteracteeComponent;
class UInteractorComponent;
class UConversationUI;
class UTicketUI;
class UModeTransitionUI;
class UConversationRuntimeComponent;
class UConversationUIBridgeComponent;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract, config="Game")
class RIOTSTORY_API ARiotStoryPlayerController : public APlayerController, public IGameplayInputTypeControllerInterface
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	ARiotStoryPlayerController();

	//~ Begin IGameplayInputTypeControllerInterface
	virtual void SetGameplayInputTypeEnabled(EGameplayInputType InputType, bool bEnabled) override;
	virtual void SetGameplayInputTypesEnabled(EGameplayInputType InputTypes, bool bEnabled) override;
	virtual void SetAllGameplayInputTypesEnabled(bool bEnabled) override;
	virtual bool IsGameplayInputTypeEnabled(EGameplayInputType InputType) const override;
	//~ End IGameplayInputTypeControllerInterface

	/** Set the current mode input type (Separate from currently enabled/disabled inputs) */
	UFUNCTION(BlueprintCallable, Category="Input")
	void SetCurrentModeInputTypes(const EGameplayInputType InputType);

	UFUNCTION(BlueprintPure, Category="Conversation")
	UConversationRuntimeComponent* GetConversationRuntime() const { return ConversationRuntime.Get(); }

	UFUNCTION(BlueprintPure, Category="Conversation")
	UConversationUIBridgeComponent* GetConversationUIBridge() const { return ConversationUIBridge.Get(); }

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Gameplay movement input mapping contexts toggled by runtime input-type state. */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MoveMappingContexts;

	/** Gameplay look input mapping contexts toggled by runtime input-type state. */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> LookMappingContexts;

	/** Gameplay combat input mapping contexts toggled by runtime input-type state. */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> CombatMappingContexts;

	/** Gameplay interact input mapping contexts toggled by runtime input-type state. */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> InteractMappingContexts;

	/** Conversation interact input mapping contexts toggle by runtime input-type state. */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> ConversationSelectionInputMappingContexts;

	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DrinkSelectionInputMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Interact Widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI|Interaction")
	TSubclassOf<UInteractUI> InteractWidgetClass;

	/** Conversation Widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI|Conversation")
	TSubclassOf<UConversationUI> ConversationWidgetClass;

	/** Ticket Widget to spawn */
	UPROPERTY(EditAnywhere, Category="UI|Ticket")
	TSubclassOf<UTicketUI> TicketWidgetClass;

	UPROPERTY(EditAnywhere, Category="UI|ModeTransition")
	TSubclassOf<UModeTransitionUI> ModeTransitionWidgetClass;

	/** UMG Widget object for the mobile UI */
	UPROPERTY(BlueprintReadOnly, Category="UI|Mobile")
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** UMG Widget object for the Interaction UI */
	UPROPERTY(BlueprintReadOnly, Category="UI|Interact")
	TObjectPtr<UInteractUI> InteractUI;

	/** UMG Widget object for the conversation UI */
	UPROPERTY(BlueprintReadOnly, Category="UI|Conversation")
	TObjectPtr<UConversationUI> ConversationUI;

	/** UMG Widget object for the ticket UI */
	UPROPERTY(BlueprintReadOnly, Category="UI|Ticket")
	TObjectPtr<UTicketUI> TicketUI;

	/** UMG Widget object for the ModeTransition UI */
	UPROPERTY(BlueprintReadOnly, Category="UI|ModeTransition")
	TObjectPtr<UModeTransitionUI> ModeTransitionUI;

	/** Conversation Component to handle conversation interactions */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UConversationRuntimeComponent> ConversationRuntime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conversation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UConversationUIBridgeComponent> ConversationUIBridge;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	//~ Begin Input Section
	bool ShouldUseTouchControls() const;
	void RefreshInputMappingContexts();
	//~ End Input Section

	virtual void LoadPlayerHUD();

	//~ Begin UI Events/Handlers/Helpers
	UFUNCTION()
	void OnConversationStateChanged(FConversationStateChangedMessage Message);

	UFUNCTION()
	void OnConversationEnded(FConversationEndedMessage Message);

	UFUNCTION()
	void OnConversationResponse(FConversationResponseEventMessage Message);

	/** A Conversation response was clicked/selected */
	UFUNCTION()
	void OnConversationResponseIndexChosen(int32 ResponseIndex);

	UFUNCTION()
	void OnConversationWindowClicked();

	UFUNCTION()
	void OnInteractableHighlightChanged(AActor* Interactor, UInteracteeComponent* PreviousInteractee, UInteracteeComponent* NewInteractee, AActor* NewTargetActor, FInteractionDisplayInfo NewDisplayInfo);

	void SyncTicketUIToCount(int32 NewTicketCount);
	//~ End UI Events/Handlers/Helpers
private:

	EGameplayInputType EnabledGameplayInputTypes = EGameplayInputType::All; //The actual current enabled input types
	EGameplayInputType CurrentGameplayInputTypes = EGameplayInputType::All; //A saved reference to a current mode subset

	FGameplayMessageListenerHandle TicketsInventoryListenHandler;

	FName TicketItemId = FName("Ticket");
	int32 LastKnownTicketCount = 0;

	TWeakObjectPtr<UInteractorComponent> BoundInteractorComponent;
private:
	void HandleItemChangedEvent(FGameplayTag Channel, const FInventoryItemChangedEvent& Message);
};
