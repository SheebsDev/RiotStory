#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "ConversationSourceComponent.generated.h"

/**
 * Provides conversation entry-point data for an interactable owner.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Conversation), meta=(BlueprintSpawnableComponent))
class RIOTSTORY_API UConversationSourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UConversationSourceComponent();

	/** The first node row to load when a talk interaction starts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conversation")
	FDataTableRowHandle StartNode;

	UFUNCTION(BlueprintPure, Category="Conversation")
	bool HasValidStartNode() const;
};

