#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionProviderInterface.generated.h"

class UPrimitiveComponent;

UINTERFACE(BlueprintType)
class RIOTSTORY_API UInteractionProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Allows an actor to explicitly provide which collision primitive should be
 * used for interaction targeting.
 */
class RIOTSTORY_API IInteractionProviderInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
	UPrimitiveComponent* GetInteractionCollisionComponent() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction")
	void ToggleInteractableHighlight(bool bHighlighted);
};

