#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardBucket.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UInterpToMovementComponent;

UCLASS(Blueprintable)
class RIOTSTORY_API ACardBucket : public AActor
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* BoxCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UInterpToMovementComponent* InterpToMovement;

public:
    ACardBucket();

    /** Start moving the Card bucket/bowl */
    UFUNCTION(BlueprintCallable)
    void BeginMoving();

    /** Stop moving the Card bucket/bowl */
    UFUNCTION(BlueprintCallable)
    void StopMoving();

    /** Local-space control points used by the InterpToMovement component. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement", meta=(MakeEditWidget=true))
    TArray<FVector> ControlPointOffsets;

    /** Movement speed in normalized alpha units per second (Duration = 1 / CycleSpeed). */
    UPROPERTY(EditAnywhere, Category="Movement")
    float CycleSpeed = 1.f;

    /** Will the bucket move back and forth between control points indefinitely. */
    UPROPERTY(EditAnywhere, Category="Movement")
    bool bInfinite = true;

    UFUNCTION(BlueprintCallable, Category="Movement")
    void SetControlPointOffsets(const TArray<FVector>& NewPoints);

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UFUNCTION()
    void OnBoxOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

private:
    void ApplyMovementSettings();
    void RebuildMovementControlPoints();
};
