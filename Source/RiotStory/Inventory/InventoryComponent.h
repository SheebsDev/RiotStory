#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InventoryComponent.generated.h"

class AActor;
class UObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryItemCountChangedSignature, FName, ItemId, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInventoryItemUsedSignature, FName, ItemId, int32, Quantity, AActor*, Consumer);
USTRUCT(BlueprintType)
struct RIOTSTORY_API FInventoryItemChangedEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<AActor> OwnerActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 OldCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 NewCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 Delta = 0;

	/** Optional related actor (for example the consumer actor during item consumption). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<AActor> RelatedActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FGameplayTag ChangeReasonTag;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChangedSignature, FInventoryItemChangedEvent, Event);

/**
 * Lightweight stack-based inventory: ItemId (FName) -> quantity.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class RIOTSTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UInventoryComponent();

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void AddItems(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveItems(FName ItemId, int32 Quantity);

	UFUNCTION(BlueprintPure, Category="Inventory")
	bool HasItems(FName ItemId, int32 Quantity) const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetItemCount(FName ItemId) const;

	/**
	 * Attempts to use Quantity of ItemId by resolving a consumer actor.
	 * PreferredConsumer is used first; if invalid/non-consumer, owner actor/components are fallback.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool TryUseItem(FName ItemId, int32 Quantity, AActor* PreferredConsumer);

	/**
	 * Attempts to use Quantity of ItemId by resolving a consumer object.
	 * Resolution order: PreferredConsumer object, owner actor, then owner components.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool TryUseItemWithConsumerObject(FName ItemId, int32 Quantity, UObject* PreferredConsumer);

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FInventoryItemCountChangedSignature OnItemCountChanged;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FInventoryItemUsedSignature OnItemUsed;

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FInventoryItemChangedSignature OnItemChanged;

private:

	/** Current inventory stacks keyed by item id. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory", meta=(AllowPrivateAccess="true"))
	TMap<FName, int32> ItemStacks;

	bool ResolveConsumerObject(UObject* PreferredConsumer, UObject*& OutConsumer) const;
	static bool IsConsumerObject(UObject* CandidateObject);
	AActor* ResolveConsumerActorForEvent(UObject* ConsumerObject) const;
	void BroadcastItemChangedEvent(FName ItemId, int32 OldCount, int32 NewCount, AActor* RelatedActor, FGameplayTag ChangeReasonTag);
	void SetItemCount(FName ItemId, int32 NewCount);

	static bool IsValidItemRequest(FName ItemId, int32 Quantity);
};
