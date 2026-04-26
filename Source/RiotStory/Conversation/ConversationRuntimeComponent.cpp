#include "Conversation/ConversationRuntimeComponent.h"
#include "Conversation/ConversationGameplayTags.h"
#include "Conversation/ConversationData.h"
#include "Interaction/InteracteeComponent.h"
#include "Inventory/InventoryComponent.h"
#include "GameFramework/PlayerController.h"

#include "Debug/RiotStoryDebugUtil.h"

UConversationRuntimeComponent::UConversationRuntimeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UConversationRuntimeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld() == nullptr)
	{
		return;
	}

	StartRequestListenerHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FConversationStartRequestMessage>(
		RiotStoryConversationTags::TAG_Message_Conversation_StartRequest,
		this,
		&UConversationRuntimeComponent::HandleConversationStartRequest
	);
}

void UConversationRuntimeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (StartRequestListenerHandle.IsValid() && GetWorld() != nullptr)
	{
		UGameplayMessageSubsystem::Get(this).UnregisterListener(StartRequestListenerHandle);
	}

	if (bConversationActive)
	{
		EndConversation(EConversationEndReason::Cancelled);
	}

	Super::EndPlay(EndPlayReason);
}

FText UConversationRuntimeComponent::GetCurrentChunkText() const
{
	if (!bConversationActive || !CurrentNodeData.Chunks.IsValidIndex(CurrentChunkIndex))
	{
		return FText::GetEmpty();
	}

	return CurrentNodeData.Chunks[CurrentChunkIndex];
}

bool UConversationRuntimeComponent::AdvanceConversation()
{
	if (!bConversationActive || bAwaitingResponse)
	{
		return false;
	}

	const int32 NextChunkIndex = CurrentChunkIndex + 1;
	if (CurrentNodeData.Chunks.IsValidIndex(NextChunkIndex))
	{
		CurrentChunkIndex = NextChunkIndex;
		PublishConversationStateChanged();
		return true;
	}

	if (CurrentNodeData.Responses.Num() > 0)
	{
		bAwaitingResponse = true;
		PublishConversationStateChanged();
		return true;
	}

	if (CurrentNodeData.NextNodeIfNoResponses != NAME_None)
	{
		if (!LoadNode(CurrentNodeData.NextNodeIfNoResponses))
		{
			EndConversation(EConversationEndReason::InvalidData);
			return false;
		}

		return true;
	}

	EndConversation(EConversationEndReason::Completed);
	return true;
}

bool UConversationRuntimeComponent::SelectConversationResponse(int32 ResponseIndex)
{
	if (!bConversationActive || !bAwaitingResponse || !CurrentNodeData.Responses.IsValidIndex(ResponseIndex))
	{
		return false;
	}

	const FConversationResponseEntry& SelectedResponse = CurrentNodeData.Responses[ResponseIndex];

	if (!TrySpendResponseCost(SelectedResponse))
	{
		return false;
	}

	PublishConversationResponseEvent(SelectedResponse, ResponseIndex);
	bAwaitingResponse = false;

	if (SelectedResponse.NextNodeId != NAME_None)
	{
		if (!LoadNode(SelectedResponse.NextNodeId))
		{
			EndConversation(EConversationEndReason::InvalidData);
			return false;
		}

		return true;
	}

	EndConversation(EConversationEndReason::Completed);
	return true;
}

void UConversationRuntimeComponent::EndConversation(EConversationEndReason Reason)
{
	if (!bConversationActive)
	{
		return;
	}

	PublishConversationEnded(Reason);
	ClearConversationState();
}

bool UConversationRuntimeComponent::StartConversationFromHandle(AActor* InInteractorActor, AActor* InSourceActor, UInteracteeComponent* InSourceInteractee, const FDataTableRowHandle& StartNodeHandle)
{
	const UDataTable* const StartDataTable = StartNodeHandle.DataTable;
	if (!IsValid(InInteractorActor) || StartDataTable == nullptr || StartNodeHandle.RowName == NAME_None)
	{
		return false;
	}

	if (bConversationActive)
	{
		EndConversation(EConversationEndReason::Replaced);
	}

	InteractorActor = InInteractorActor;
	SourceActor = InSourceActor;
	SourceInteractee = InSourceInteractee;
	ActiveConversationTable = const_cast<UDataTable*>(StartDataTable);

	if (!LoadNode(StartNodeHandle.RowName))
	{
		PublishConversationEnded(EConversationEndReason::FailedToStart);
		ClearConversationState();
		return false;
	}

	return true;
}

void UConversationRuntimeComponent::HandleConversationStartRequest(FGameplayTag Channel, const FConversationStartRequestMessage& Message)
{
	if (!DoesRequestMatchOwningInteractor(Message))
	{
		return;
	}

	StartConversationFromHandle(Message.InteractorActor.Get(), Message.SourceActor.Get(), Message.SourceInteractee.Get(), Message.StartNode);
}

bool UConversationRuntimeComponent::DoesRequestMatchOwningInteractor(const FConversationStartRequestMessage& Message) const
{
	if (!IsValid(Message.InteractorActor))
	{
		return false;
	}

	if (APlayerController* const OwnerPC = GetOwningPlayerController())
	{
		return OwnerPC->GetPawn() == Message.InteractorActor.Get();
	}

	return false;
}

APlayerController* UConversationRuntimeComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

bool UConversationRuntimeComponent::LoadNode(const FName NodeId)
{
	if (ActiveConversationTable == nullptr || NodeId == NAME_None)
	{
		return false;
	}

	const FConversationNodeRow* const NodeRow = ActiveConversationTable->FindRow<FConversationNodeRow>(NodeId, TEXT("ConversationLoadNode"));
	if (NodeRow == nullptr || NodeRow->Chunks.Num() == 0)
	{
		return false;
	}

	CurrentNodeData = *NodeRow;
	ActiveNodeId = NodeId;
	CurrentChunkIndex = 0;
	bConversationActive = true;
	bAwaitingResponse = false;

	PublishConversationStateChanged();
	return true;
}

void UConversationRuntimeComponent::ClearConversationState()
{
	bConversationActive = false;
	bAwaitingResponse = false;
	InteractorActor = nullptr;
	SourceActor = nullptr;
	SourceInteractee = nullptr;
	ActiveConversationTable = nullptr;
	ActiveNodeId = NAME_None;
	CurrentChunkIndex = INDEX_NONE;
	CurrentNodeData = FConversationNodeRow();
}

void UConversationRuntimeComponent::PublishConversationStateChanged() const
{
	if (GetWorld() == nullptr || !bConversationActive)
	{
		return;
	}

	FConversationStateChangedMessage StateMessage;
	StateMessage.InteractorActor = InteractorActor;
	StateMessage.SourceActor = SourceActor;
	StateMessage.SourceInteractee = SourceInteractee;
	StateMessage.ActiveNodeId = ActiveNodeId;
	StateMessage.SpeakerDisplayName = ResolveSpeakerDisplayName();
	StateMessage.CurrentChunkText = GetCurrentChunkText();
	StateMessage.CurrentChunkIndex = CurrentChunkIndex;
	StateMessage.TotalChunks = CurrentNodeData.Chunks.Num();
	StateMessage.bAwaitingResponse = bAwaitingResponse;
	StateMessage.bCanAdvance = !bAwaitingResponse;

	if (bAwaitingResponse)
	{
		for (int32 Index = 0; Index < CurrentNodeData.Responses.Num(); ++Index)
		{
			const FConversationResponseEntry& Response = CurrentNodeData.Responses[Index];
			FConversationResponseOptionDisplay& DisplayOption = StateMessage.ResponseOptions.AddDefaulted_GetRef();
			DisplayOption.ResponseIndex = Index;
			DisplayOption.ResponseText = Response.ResponseText;
			DisplayOption.CostItemId = Response.CostItemId;
			DisplayOption.CostQuantity = Response.CostQuantity;
		}
	}

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		RiotStoryConversationTags::TAG_Message_Conversation_StateChanged,
		StateMessage
	);
}

void UConversationRuntimeComponent::PublishConversationEnded(EConversationEndReason Reason) const
{
	if (GetWorld() == nullptr)
	{
		return;
	}

	FConversationEndedMessage EndedMessage;
	EndedMessage.InteractorActor = InteractorActor;
	EndedMessage.SourceActor = SourceActor;
	EndedMessage.SourceInteractee = SourceInteractee;
	EndedMessage.Reason = Reason;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		RiotStoryConversationTags::TAG_Message_Conversation_Ended,
		EndedMessage
	);
}

void UConversationRuntimeComponent::PublishConversationResponseEvent(const FConversationResponseEntry& SelectedResponse, int32 ResponseIndex) const
{
	if (GetWorld() == nullptr || !SelectedResponse.ResponseEventTag.IsValid())
	{
		return;
	}

	FConversationResponseEventMessage ResponseMessage;
	ResponseMessage.InteractorActor = InteractorActor;
	ResponseMessage.SourceActor = SourceActor;
	ResponseMessage.SourceInteractee = SourceInteractee;
	ResponseMessage.SelectedResponseIndex = ResponseIndex;
	ResponseMessage.ResponseEventTag = SelectedResponse.ResponseEventTag;
	ResponseMessage.CostItemId = SelectedResponse.CostItemId;
	ResponseMessage.CostQuantity = SelectedResponse.CostQuantity;

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		RiotStoryConversationTags::TAG_Message_Conversation_ResponseEvent,
		ResponseMessage
	);
}

bool UConversationRuntimeComponent::TrySpendResponseCost(const FConversationResponseEntry& SelectedResponse) const
{
	if (SelectedResponse.CostItemId.IsNone() || SelectedResponse.CostQuantity <= 0)
	{
		return true;
	}

	if (!IsValid(InteractorActor))
	{
		return false;
	}

	UInventoryComponent* const Inventory = InteractorActor->FindComponentByClass<UInventoryComponent>();
	if (!IsValid(Inventory))
	{
		return false;
	}

	return Inventory->TryUseItemWithConsumerObject(SelectedResponse.CostItemId, SelectedResponse.CostQuantity, nullptr);
}

FText UConversationRuntimeComponent::ResolveSpeakerDisplayName() const
{
	if (!CurrentNodeData.SpeakerDisplayName.IsEmpty())
	{
		return CurrentNodeData.SpeakerDisplayName;
	}

	if (IsValid(SourceActor))
	{
		return FText::FromString(SourceActor->GetName());
	}

	return FText::FromString(TEXT("Speaker"));
}
