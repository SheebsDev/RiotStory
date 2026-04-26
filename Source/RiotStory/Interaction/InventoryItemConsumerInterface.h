#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryItemConsumerInterface.generated.h"

class AActor;

UINTERFACE(BlueprintType)
class RIOTSTORY_API UInventoryItemConsumerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Contract for objects that can consume items from an interactor's inventory.
 */
class RIOTSTORY_API IInventoryItemConsumerInterface
{
	GENERATED_BODY()

public:

	/** Returns true if this actor can consume the requested item quantity. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Inventory")
	bool CanConsumeItem(AActor* Interactor, FName ItemId, int32 Quantity);

	/** Notifies this actor that the requested item quantity was consumed. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Inventory")
	void ConsumeItem(AActor* Interactor, FName ItemId, int32 Quantity);
};
