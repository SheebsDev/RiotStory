#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "Interaction/InteracteeTypes.h"
#include "Interaction/InventoryItemConsumerInterface.h"
#include "InteracteeComponent.generated.h"

class AActor;
struct FConversationStartRequestMessage;
class UPrimitiveComponent;

UENUM(BlueprintType)
enum class EInteracteeMode : uint8
{
	Loot,
	Vendor,
	Custom
};

/**
 * Component-first interaction contract and default loot/vendor implementation.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class RIOTSTORY_API UInteracteeComponent : public UActorComponent, public IInventoryItemConsumerInterface
{
	GENERATED_BODY()

public:

	UInteracteeComponent();

	virtual void OnRegister() override;
	virtual void BeginPlay() override;

	/** Behavior mode used by the default native implementation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	EInteracteeMode InteractionMode = EInteracteeMode::Custom;

	/** UI data displayed while this interactee is highlighted. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Display")
	FInteractionDisplayInfo DisplayInfo;

	/** Semantic action/verb used by systems such as conversation triggering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	FGameplayTag InteractionActionTag;

	/** Collision channel this interactee should block to be detected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Trace")
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_GameTraceChannel2;

	/** If true, the resolved collision primitive is configured to block InteractionTraceChannel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Trace")
	bool bAutoConfigureTraceCollision = true;

	/** Optional explicit collision primitive used for interaction targeting. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Interaction|Collision")
	TObjectPtr<UPrimitiveComponent> ExplicitInteractionCollision;

	/** Item id granted by loot interactions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loot")
	FName ItemId = FName("Ticket");

	/** Quantity granted by loot interactions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loot", meta=(ClampMin=1))
	int32 Quantity = 1;

	/** Destroy owning actor after successful loot interaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loot")
	bool bDestroyOwnerAfterLoot = true;

	/** Hide and disable owning actor after successful loot interaction when not destroyed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Loot")
	bool bDisableOwnerAfterLoot = false;

	/** Item id required by vendor interactions. None means no cost. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vendor|Cost")
	FName RequiredItemId = FName("Credits");

	/** Item quantity required by vendor interactions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vendor|Cost", meta=(ClampMin=1))
	int32 RequiredQuantity = 1;

	/** If true, vendor interactions consume required items on success. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vendor|Cost")
	bool bConsumeRequiredItems = true;

	/** Master toggle allowing this interactee to consume items. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vendor|Behavior")
	bool bCanConsumeItems = true;

	/** Determines whether this interactee can be used by Interactor. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
	bool CanInteract(AActor* Interactor);

	/** Executes interaction and returns success. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
	bool ExecuteInteraction(AActor* Interactor);

	/** Builds UI prompt data for this interactee. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Interaction")
	bool GetInteractionDisplayInfo(AActor* Interactor, FInteractionDisplayInfo& OutDisplayInfo) const;

	/** Returns currently resolved collision primitive, if any. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Interaction")
	UPrimitiveComponent* GetInteractionCollisionComponent() const { return ResolvedInteractionCollision.Get(); }

	/** Returns true if this interactee is bound to the provided collision primitive. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Interaction")
	bool MatchesCollisionComponent(const UPrimitiveComponent* CollisionComponent) const;

	/** Resolves and (optionally) configures the collision primitive used for interaction traces. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void RefreshInteractionCollisionBinding();

	/** Returns true if this interactee has a valid resolved collision primitive. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Interaction")
	bool IsInteractionEnabled() const;

	/** Returns true when vendor cost fields are valid. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Vendor|Cost")
	bool HasValidCostRequirement() const;

	/** Returns true when this interaction should trigger conversation flow. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Interaction")
	bool IsTalkingInteraction() const;

	UFUNCTION(BlueprintCallable, Category="Interaction")
	void ToggleHighlight(bool bHighlighted);

	//~Begin IInventoryItemConsumerInterface
	virtual bool CanConsumeItem_Implementation(AActor* Interactor, FName ItemIdToConsume, int32 QuantityToConsume) override;
	virtual void ConsumeItem_Implementation(AActor* Interactor, FName ItemIdToConsume, int32 QuantityToConsume) override;
	//~End IInventoryItemConsumerInterface

protected:

	virtual bool CanInteract_Implementation(AActor* Interactor);
	virtual bool ExecuteInteraction_Implementation(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category="Loot", meta=(DisplayName="On Loot Collected"))
	void BP_OnLootCollected(AActor* Interactor, FName GrantedItemId, int32 GrantedQuantity);

	UFUNCTION(BlueprintImplementableEvent, Category="Vendor", meta=(DisplayName="On Trade Succeeded"))
	void BP_OnTradeSucceeded(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category="Vendor", meta=(DisplayName="On Trade Failed"))
	void BP_OnTradeFailed(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, Category="Vendor", meta=(DisplayName="On Item Consumed"))
	void BP_OnItemConsumed(AActor* Interactor, FName ConsumedItemId, int32 ConsumedQuantity);

private:

	UPROPERTY(Transient)
	TObjectPtr<UPrimitiveComponent> ResolvedInteractionCollision;

	bool ResolveInteractionCollisionComponent(UPrimitiveComponent*& OutCollisionComponent) const;
	bool IsValidInteractionCollisionComponent(const UPrimitiveComponent* CollisionComponent) const;
	void ConfigureCollisionForInteraction(UPrimitiveComponent* CollisionComponent) const;
	bool TryBuildConversationStartRequest(AActor* Interactor, FConversationStartRequestMessage& OutRequestMessage) const;

	bool HandleLootInteraction(AActor* Interactor);
	bool HandleVendorInteraction(AActor* Interactor);
};
