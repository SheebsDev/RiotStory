#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ConversationMessages.generated.h"

class AActor;
class UInteracteeComponent;

UENUM(BlueprintType)
enum class EConversationEndReason : uint8
{
	Completed,
	Cancelled,
	Replaced,
	InvalidData,
	FailedToStart
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FConversationResponseOptionDisplay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	int32 ResponseIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FText ResponseText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation|Cost")
	FName CostItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation|Cost")
	int32 CostQuantity = 0;
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FConversationStartRequestMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<AActor> InteractorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<UInteracteeComponent> SourceInteractee = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FDataTableRowHandle StartNode;
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FConversationStateChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<AActor> InteractorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<UInteracteeComponent> SourceInteractee = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FName ActiveNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FText SpeakerDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FText CurrentChunkText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	int32 CurrentChunkIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	int32 TotalChunks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	bool bAwaitingResponse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	bool bCanAdvance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TArray<FConversationResponseOptionDisplay> ResponseOptions;
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FConversationEndedMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<AActor> InteractorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<UInteracteeComponent> SourceInteractee = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	EConversationEndReason Reason = EConversationEndReason::Completed;
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FConversationResponseEventMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<AActor> InteractorActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TObjectPtr<UInteracteeComponent> SourceInteractee = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	int32 SelectedResponseIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FGameplayTag ResponseEventTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation|Cost")
	FName CostItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation|Cost")
	int32 CostQuantity = 0;
};

