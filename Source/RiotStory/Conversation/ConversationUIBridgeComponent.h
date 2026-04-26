#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Conversation/ConversationMessages.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ConversationUIBridgeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConversationStateChangedSignature, FConversationStateChangedMessage, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConversationEndedSignature, FConversationEndedMessage, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConversationResponseEventSignature, FConversationResponseEventMessage, Message);

/**
 * Controller-side message bridge for conversation UI widgets.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Conversation), meta=(BlueprintSpawnableComponent))
class RIOTSTORY_API UConversationUIBridgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UConversationUIBridgeComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintAssignable, Category="Conversation|UI")
	FConversationStateChangedSignature OnConversationStateChanged;

	UPROPERTY(BlueprintAssignable, Category="Conversation|UI")
	FConversationEndedSignature OnConversationEnded;

	UPROPERTY(BlueprintAssignable, Category="Conversation|UI")
	FConversationResponseEventSignature OnConversationResponseEvent;

private:

	FGameplayMessageListenerHandle StateChangedListenerHandle;
	FGameplayMessageListenerHandle EndedListenerHandle;
	FGameplayMessageListenerHandle ResponseEventListenerHandle;

	void HandleStateChangedMessage(FGameplayTag Channel, const FConversationStateChangedMessage& Message);
	void HandleEndedMessage(FGameplayTag Channel, const FConversationEndedMessage& Message);
	void HandleResponseEventMessage(FGameplayTag Channel, const FConversationResponseEventMessage& Message);

	bool IsMessageForOwningInteractor(const AActor* InteractorActor) const;
};
