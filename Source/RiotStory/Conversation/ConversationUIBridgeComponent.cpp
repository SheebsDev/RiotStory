#include "Conversation/ConversationUIBridgeComponent.h"
#include "Conversation/ConversationGameplayTags.h"
#include "GameFramework/PlayerController.h"

UConversationUIBridgeComponent::UConversationUIBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UConversationUIBridgeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld() == nullptr)
	{
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);

	StateChangedListenerHandle = MessageSubsystem.RegisterListener<FConversationStateChangedMessage>(
		RiotStoryConversationTags::TAG_Message_Conversation_StateChanged,
		this,
		&UConversationUIBridgeComponent::HandleStateChangedMessage
	);

	EndedListenerHandle = MessageSubsystem.RegisterListener<FConversationEndedMessage>(
		RiotStoryConversationTags::TAG_Message_Conversation_Ended,
		this,
		&UConversationUIBridgeComponent::HandleEndedMessage
	);

	ResponseEventListenerHandle = MessageSubsystem.RegisterListener<FConversationResponseEventMessage>(
		RiotStoryConversationTags::TAG_Message_Conversation_ResponseEvent,
		this,
		&UConversationUIBridgeComponent::HandleResponseEventMessage
	);
}

void UConversationUIBridgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld() != nullptr)
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);

		if (StateChangedListenerHandle.IsValid())
		{
			MessageSubsystem.UnregisterListener(StateChangedListenerHandle);
		}

		if (EndedListenerHandle.IsValid())
		{
			MessageSubsystem.UnregisterListener(EndedListenerHandle);
		}

		if (ResponseEventListenerHandle.IsValid())
		{
			MessageSubsystem.UnregisterListener(ResponseEventListenerHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UConversationUIBridgeComponent::HandleStateChangedMessage(FGameplayTag Channel, const FConversationStateChangedMessage& Message)
{
	(void)Channel;

	if (!IsMessageForOwningInteractor(Message.InteractorActor))
	{
		return;
	}

	OnConversationStateChanged.Broadcast(Message);
}

void UConversationUIBridgeComponent::HandleEndedMessage(FGameplayTag Channel, const FConversationEndedMessage& Message)
{
	(void)Channel;

	if (!IsMessageForOwningInteractor(Message.InteractorActor))
	{
		return;
	}

	OnConversationEnded.Broadcast(Message);
}

void UConversationUIBridgeComponent::HandleResponseEventMessage(FGameplayTag Channel, const FConversationResponseEventMessage& Message)
{
	(void)Channel;

	if (!IsMessageForOwningInteractor(Message.InteractorActor))
	{
		return;
	}

	OnConversationResponseEvent.Broadcast(Message);
}

bool UConversationUIBridgeComponent::IsMessageForOwningInteractor(const AActor* InteractorActor) const
{
	const APlayerController* const OwnerPC = Cast<APlayerController>(GetOwner());
	return IsValid(OwnerPC) && OwnerPC->GetPawn() == InteractorActor;
}

