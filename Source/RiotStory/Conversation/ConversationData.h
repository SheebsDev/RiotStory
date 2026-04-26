#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ConversationData.generated.h"

USTRUCT(BlueprintType)
struct RIOTSTORY_API FConversationResponseEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FText ResponseText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FName NextNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation|Cost")
	FName CostItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation|Cost", meta=(ClampMin=0))
	int32 CostQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation|Events")
	FGameplayTag ResponseEventTag;
};

USTRUCT(BlueprintType)
struct RIOTSTORY_API FConversationNodeRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FText SpeakerDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TArray<FText> Chunks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	TArray<FConversationResponseEntry> Responses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FName NextNodeIfNoResponses = NAME_None;
};

