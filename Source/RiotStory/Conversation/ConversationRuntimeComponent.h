#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Conversation/ConversationData.h"
#include "Conversation/ConversationMessages.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ConversationRuntimeComponent.generated.h"

class AActor;
class APlayerController;
class UDataTable;
class UInteracteeComponent;

/**
 * Local conversation state machine for a player.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Conversation), meta=(BlueprintSpawnableComponent))
class RIOTSTORY_API UConversationRuntimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UConversationRuntimeComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category="Conversation")
	bool IsConversationActive() const { return bConversationActive; }

	UFUNCTION(BlueprintPure, Category="Conversation")
	bool IsAwaitingResponse() const { return bAwaitingResponse; }

	UFUNCTION(BlueprintPure, Category="Conversation")
	FName GetActiveNodeId() const { return ActiveNodeId; }

	UFUNCTION(BlueprintPure, Category="Conversation")
	int32 GetCurrentChunkIndex() const { return CurrentChunkIndex; }

	UFUNCTION(BlueprintPure, Category="Conversation")
	FText GetCurrentChunkText() const;

	/** Advance one chunk/state transition when a conversation is active. */
	UFUNCTION(BlueprintCallable, Category="Conversation")
	bool AdvanceConversation();

	/** Select one of the currently offered response options. */
	UFUNCTION(BlueprintCallable, Category="Conversation")
	bool SelectConversationResponse(int32 ResponseIndex);

	/** Force-end the current conversation. */
	UFUNCTION(BlueprintCallable, Category="Conversation")
	void EndConversation(EConversationEndReason Reason);

	/** Starts a conversation directly from a row handle. */
	UFUNCTION(BlueprintCallable, Category="Conversation")
	bool StartConversationFromHandle(AActor* InInteractorActor, AActor* InSourceActor, UInteracteeComponent* InSourceInteractee, const FDataTableRowHandle& StartNodeHandle);

private:

	UPROPERTY(Transient)
	bool bConversationActive = false;

	UPROPERTY(Transient)
	bool bAwaitingResponse = false;

	UPROPERTY(Transient)
	TObjectPtr<AActor> InteractorActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UInteracteeComponent> SourceInteractee = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> ActiveConversationTable = nullptr;

	UPROPERTY(Transient)
	FName ActiveNodeId = NAME_None;

	UPROPERTY(Transient)
	int32 CurrentChunkIndex = INDEX_NONE;

	UPROPERTY(Transient)
	FConversationNodeRow CurrentNodeData;

	FGameplayMessageListenerHandle StartRequestListenerHandle;

	void HandleConversationStartRequest(FGameplayTag Channel, const FConversationStartRequestMessage& Message);

	bool DoesRequestMatchOwningInteractor(const FConversationStartRequestMessage& Message) const;
	APlayerController* GetOwningPlayerController() const;

	bool LoadNode(const FName NodeId);
	void ClearConversationState();

	void PublishConversationStateChanged() const;
	void PublishConversationEnded(EConversationEndReason Reason) const;
	void PublishConversationResponseEvent(const FConversationResponseEntry& SelectedResponse, int32 ResponseIndex) const;

	bool TrySpendResponseCost(const FConversationResponseEntry& SelectedResponse) const;
	FText ResolveSpeakerDisplayName() const;
};
