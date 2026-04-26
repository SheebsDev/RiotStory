// Copyright Epic Games, Inc. All Rights Reserved.


#include "RiotStoryPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Conversation/ConversationRuntimeComponent.h"
#include "Conversation/ConversationUIBridgeComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "RiotStoryCameraManager.h"
#include "Blueprint/UserWidget.h"
#include "RiotStory.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Interaction/InteractorComponent.h"
#include "Interaction/InteracteeComponent.h"
#include "UI/InteractUI.h"
#include "UI/ConversationUI.h"
#include "UI/TicketUI.h"
#include "UI/ModeTransitionUI.h"
#include "Inventory/InventoryGameplayTags.h"
#include "Game/RiotStoryGameplayTags.h"

namespace
{
void RemoveMappingContexts(UEnhancedInputLocalPlayerSubsystem* const Subsystem, const TArray<UInputMappingContext*>& MappingContexts)
{
	if (!IsValid(Subsystem))
	{
		return;
	}

	for (UInputMappingContext* const MappingContext : MappingContexts)
	{
		if (IsValid(MappingContext))
		{
			Subsystem->RemoveMappingContext(MappingContext);
		}
	}
}

void AddMappingContexts(UEnhancedInputLocalPlayerSubsystem* const Subsystem, const TArray<UInputMappingContext*>& MappingContexts, const int32 Priority)
{
	if (!IsValid(Subsystem))
	{
		return;
	}

	for (UInputMappingContext* const MappingContext : MappingContexts)
	{
		if (IsValid(MappingContext))
		{
			Subsystem->AddMappingContext(MappingContext, Priority);
		}
	}
}
}

ARiotStoryPlayerController::ARiotStoryPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = ARiotStoryCameraManager::StaticClass();

	ConversationRuntime = CreateDefaultSubobject<UConversationRuntimeComponent>(TEXT("Conversation Runtime"));
	ConversationUIBridge = CreateDefaultSubobject<UConversationUIBridgeComponent>(TEXT("Conversation UI Bridge"));
}

void ARiotStoryPlayerController::SetGameplayInputTypeEnabled(const EGameplayInputType InputType, const bool bEnabled)
{
	SetGameplayInputTypesEnabled(InputType, bEnabled);
}

void ARiotStoryPlayerController::SetGameplayInputTypesEnabled(const EGameplayInputType InputTypes, const bool bEnabled)
{
	if (InputTypes == EGameplayInputType::None)
	{
		return;
	}

	if (bEnabled)
	{
		EnabledGameplayInputTypes |= InputTypes;
	}
	else
	{
		EnabledGameplayInputTypes &= ~InputTypes;
	}

	RefreshInputMappingContexts();
}

void ARiotStoryPlayerController::SetAllGameplayInputTypesEnabled(const bool bEnabled)
{
	EnabledGameplayInputTypes = bEnabled ? EGameplayInputType::All : EGameplayInputType::None;
	RefreshInputMappingContexts();
}

bool ARiotStoryPlayerController::IsGameplayInputTypeEnabled(const EGameplayInputType InputType) const
{
	return InputType != EGameplayInputType::None && EnumHasAllFlags(EnabledGameplayInputTypes, InputType);
}

void ARiotStoryPlayerController::SetCurrentModeInputTypes(const EGameplayInputType InputTypes)
{
	CurrentGameplayInputTypes = InputTypes;
	SetAllGameplayInputTypesEnabled(false);
	SetGameplayInputTypesEnabled(InputTypes, true);
}

void ARiotStoryPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		LoadPlayerHUD();
	}

	ConversationUIBridge->OnConversationStateChanged.AddDynamic(this, &ARiotStoryPlayerController::OnConversationStateChanged);
	ConversationUIBridge->OnConversationEnded.AddDynamic(this, &ARiotStoryPlayerController::OnConversationEnded);
	ConversationUIBridge->OnConversationResponseEvent.AddDynamic(this, &ARiotStoryPlayerController::OnConversationResponse);
}

void ARiotStoryPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//Cleanup Listeners
	UGameplayMessageSubsystem::Get(this).UnregisterListener(TicketsInventoryListenHandler);
}

void ARiotStoryPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UInteractorComponent* const ExistingInteractor = BoundInteractorComponent.Get())
	{
		ExistingInteractor->OnInteractableHighlightChanged.RemoveDynamic(this, &ARiotStoryPlayerController::OnInteractableHighlightChanged);
		BoundInteractorComponent.Reset();
	}

	if (!IsValid(InPawn))
	{
		return;
	}

	if (UInteractorComponent* const InteractorComponent = InPawn->FindComponentByClass<UInteractorComponent>())
	{
		InteractorComponent->OnInteractableHighlightChanged.AddUniqueDynamic(this, &ARiotStoryPlayerController::OnInteractableHighlightChanged);
		BoundInteractorComponent = InteractorComponent;
	}
}

void ARiotStoryPlayerController::OnUnPossess()
{
	if (UInteractorComponent* const InteractorComponent = BoundInteractorComponent.Get())
	{
		InteractorComponent->OnInteractableHighlightChanged.RemoveDynamic(this, &ARiotStoryPlayerController::OnInteractableHighlightChanged);
		BoundInteractorComponent.Reset();
	}

	if (IsValid(InteractUI))
	{
		if (InteractUI->IsInViewport())
		{
			InteractUI->RemoveFromParent();
		}

		InteractUI->BP_HideInteract();
	}

	SyncTicketUIToCount(0);
	Super::OnUnPossess();
}

void ARiotStoryPlayerController::OnConversationStateChanged(FConversationStateChangedMessage Message)
{
	//Show and update the conversation UI
	ConversationUI->AddToViewport(0);
	ConversationUI->BP_ShowConversation(Message.CurrentChunkText, !Message.bCanAdvance);
	if (Message.ResponseOptions.Num() > 0)
	{
		ConversationUI->BP_ShowResponses(Message.ResponseOptions);
	}

	SetInputMode(FInputModeGameAndUI());
	SetShowMouseCursor(true);
	bEnableMouseOverEvents = true;

	const EGameplayInputType DisabledDuringConversation = EGameplayInputType::Move | EGameplayInputType::Look | EGameplayInputType::Combat;
	SetGameplayInputTypesEnabled(DisabledDuringConversation, false);
	SetGameplayInputTypeEnabled(EGameplayInputType::Interact, true);
}

void ARiotStoryPlayerController::OnConversationEnded(FConversationEndedMessage Message)
{
	ConversationUI->RemoveFromParent();
	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
	bEnableMouseOverEvents = false;

	//This should be a return back to the current state of input
	//That way if something changes the current state in the middle of the conversation to trigger and event then we will return to that state
	SetAllGameplayInputTypesEnabled(false);
	SetGameplayInputTypesEnabled(CurrentGameplayInputTypes, true);
}

void ARiotStoryPlayerController::OnConversationResponse(FConversationResponseEventMessage Message)
{
	ConversationUI->BP_ClearOutResponses();
}

void ARiotStoryPlayerController::OnConversationResponseIndexChosen(const int32 ResponseIndex)
{
	if (ConversationRuntime->IsConversationActive())
	{
		ConversationRuntime->SelectConversationResponse(ResponseIndex);
	}
}

void ARiotStoryPlayerController::OnConversationWindowClicked()
{
	if (ConversationRuntime->IsConversationActive())
	{
		ConversationRuntime->AdvanceConversation();
	}
}

void ARiotStoryPlayerController::OnInteractableHighlightChanged(AActor* Interactor, UInteracteeComponent* PreviousInteractee, UInteracteeComponent* NewInteractee, AActor* NewTargetActor, FInteractionDisplayInfo NewDisplayInfo)
{
	if (!IsValid(InteractUI))
	{
		return;
	}

	if (NewInteractee)
	{
		if (!InteractUI->IsInViewport())
		{
			InteractUI->AddToViewport(0);
		}

		const FText InteractText = FText::Format(
			NSLOCTEXT("Interaction", "InteractPromptFormat", "{0} {1}"),
			NewDisplayInfo.ActionText,
			NewDisplayInfo.DisplayName);
		InteractUI->BP_UpdateInteractText(InteractText);
		InteractUI->BP_ShowInteract();
	}
	else
	{
		if (InteractUI->IsInViewport())
		{
			InteractUI->RemoveFromParent();
		}

		InteractUI->BP_HideInteract();
	}
}

void ARiotStoryPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	RefreshInputMappingContexts();
}

void ARiotStoryPlayerController::RefreshInputMappingContexts()
{
	// only manage IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			RemoveMappingContexts(Subsystem, DefaultMappingContexts);
			RemoveMappingContexts(Subsystem, MobileExcludedMappingContexts);
			RemoveMappingContexts(Subsystem, MoveMappingContexts);
			RemoveMappingContexts(Subsystem, LookMappingContexts);
			RemoveMappingContexts(Subsystem, CombatMappingContexts);
			RemoveMappingContexts(Subsystem, InteractMappingContexts);
			RemoveMappingContexts(Subsystem, ConversationSelectionInputMappingContexts);

			AddMappingContexts(Subsystem, DefaultMappingContexts, 0);

			if (!ShouldUseTouchControls())
			{
				AddMappingContexts(Subsystem, MobileExcludedMappingContexts, 0);
			}

			if (IsGameplayInputTypeEnabled(EGameplayInputType::Move))
			{
				AddMappingContexts(Subsystem, MoveMappingContexts, 0);
			}

			if (IsGameplayInputTypeEnabled(EGameplayInputType::Look))
			{
				AddMappingContexts(Subsystem, LookMappingContexts, 0);
			}

			if (IsGameplayInputTypeEnabled(EGameplayInputType::Combat))
			{
				AddMappingContexts(Subsystem, CombatMappingContexts, 0);
			}

			if (IsGameplayInputTypeEnabled(EGameplayInputType::Interact))
			{
				AddMappingContexts(Subsystem, InteractMappingContexts, 0);
				AddMappingContexts(Subsystem, ConversationSelectionInputMappingContexts, 0);
			}
		}
	}
}

bool ARiotStoryPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void ARiotStoryPlayerController::LoadPlayerHUD()
{
	//Mobile UI if needed
	if (ShouldUseTouchControls())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogRiotStory, Error, TEXT("Could not spawn mobile controls widget."));
		}
		return; //NOTE:No need to build non mobile UI
	}

	if (InteractWidgetClass)
	{
		InteractUI = CreateWidget<UInteractUI>(this, InteractWidgetClass);
	}

	if (ConversationWidgetClass)
	{
		ConversationUI = CreateWidget<UConversationUI>(this, ConversationWidgetClass);

		if (ConversationUI)
		{
			ConversationUI->OnResponseIndexChosen.AddDynamic(this, &ARiotStoryPlayerController::OnConversationResponseIndexChosen);
			ConversationUI->OnConversationWindowClicked.AddDynamic(this, &ARiotStoryPlayerController::OnConversationWindowClicked);
		}
	}

	if (TicketWidgetClass)
	{
		TicketUI = CreateWidget<UTicketUI>(this, TicketWidgetClass);

		if (TicketUI)
		{
			TicketUI->AddToViewport(0);
			// Newly created widget starts empty, so replay up to the current tracked count.
			LastKnownTicketCount = 0;

			TicketsInventoryListenHandler = UGameplayMessageSubsystem::Get(this).RegisterListener<FInventoryItemChangedEvent>(
				RiotStoryInventoryTags::TAG_Message_Inventory_ItemChanged,
				this,
				&ARiotStoryPlayerController::HandleItemChangedEvent
			);
		}
	}
}

void ARiotStoryPlayerController::HandleItemChangedEvent(FGameplayTag Channel, const FInventoryItemChangedEvent& Message)
{
	if (Message.ItemId != TicketItemId)
	{
		return;
	}

	SyncTicketUIToCount(Message.NewCount);
}

void ARiotStoryPlayerController::SyncTicketUIToCount(int32 NewTicketCount)
{
	NewTicketCount = FMath::Max(0, NewTicketCount);

	if (!IsValid(TicketUI))
	{
		LastKnownTicketCount = NewTicketCount;
		return;
	}

	const int32 Delta = NewTicketCount - LastKnownTicketCount;
	if (Delta > 0)
	{
		for (int32 Index = 0; Index < Delta; ++Index)
		{
			TicketUI->BP_AddTicket();
		}
	}
	else if (Delta < 0)
	{
		for (int32 Index = 0; Index < -Delta; ++Index)
		{
			TicketUI->BP_RemoveTicket();
		}
	}

	LastKnownTicketCount = NewTicketCount;
}
