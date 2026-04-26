#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/InteractionProviderInterface.h"
#include "Interaction/InventoryItemConsumerInterface.h"
#include "VendorCharacter.generated.h"

class UInteracteeComponent;
class UConversationSourceComponent;
class UPrimitiveComponent;
class USphereComponent;
class AActor;

/**
 * Character host for vendor interactions, driven by UInteracteeComponent.
 */
UCLASS(Blueprintable)
class RIOTSTORY_API AVendorCharacter : public ACharacter, public IInteractionProviderInterface, public IInventoryItemConsumerInterface
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> InteractionCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInteracteeComponent> Interactee;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UConversationSourceComponent> ConversationSource;

public:

	AVendorCharacter();

	UFUNCTION(BlueprintPure, Category="Interaction")
	UInteracteeComponent* GetInteracteeComponent() const { return Interactee.Get(); }

	UFUNCTION(BlueprintPure, Category="Conversation")
	UConversationSourceComponent* GetConversationSourceComponent() const { return ConversationSource.Get(); }

	//~Begin IInteractionProviderInterface
	virtual UPrimitiveComponent* GetInteractionCollisionComponent_Implementation() const override;
	virtual void ToggleInteractableHighlight_Implementation(bool bHighlighted) override;
	//~End IInteractionProviderInterface

	//~Begin IInventoryItemConsumerInterface
	virtual bool CanConsumeItem_Implementation(AActor* Interactor, FName ItemId, int32 Quantity) override;
	virtual void ConsumeItem_Implementation(AActor* Interactor, FName ItemId, int32 Quantity) override;
	//~End IInventoryItemConsumerInterface

private:
	bool bIsHighlighted = false;
};
