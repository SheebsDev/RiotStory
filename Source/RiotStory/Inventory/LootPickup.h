#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractionProviderInterface.h"
#include "LootPickup.generated.h"

class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UInteracteeComponent;
class UPrimitiveComponent;

/**
 * Lightweight loot actor host using UInteracteeComponent for behavior.
 */
UCLASS(Blueprintable)
class RIOTSTORY_API ALootPickup : public AActor, public IInteractionProviderInterface
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> InteractionCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInteracteeComponent> Interactee;

public:

	ALootPickup();

	UFUNCTION(BlueprintPure, Category="Interaction")
	UInteracteeComponent* GetInteracteeComponent() const { return Interactee.Get(); }

	//~Begin IInteractionCollisionProviderInterface
	virtual UPrimitiveComponent* GetInteractionCollisionComponent_Implementation() const override;
	virtual void ToggleInteractableHighlight_Implementation(bool bHighlighted) override;
	//~End IInteractionCollisionProviderInterface

private:
	bool bIsHighlighted = false;
};

