#pragma once

#include "CoreMinimal.h"
#include "CardBucket.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
#if WITH_EDITOR
struct FPropertyChangedEvent;
#endif

UCLASS(Blueprintable)
class RIOTSTORY_API ACardBucket : public AActor
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UBoxComponent* BoxCollision;

public:
    ACardBucket();

    virtual void Tick( float DeltaSeconds ) override;

    /** Start moving the Card bucket/bowl */
    UFUNCTION(BlueprintCallable)
    void BeginMoving();

    /** Stop moving the Card bucket/bowl */
    UFUNCTION(BlueprintCallable)
    void StopMoving();

    /** The starting position where the bucket will be */
    UPROPERTY(EditAnywhere, Category="Movement")
    FVector StartPosition;

    /** The end/second position where the bucket will move */
    UPROPERTY(EditAnywhere, Category="Movement")
    FVector EndPosition;

    /** Speed it takes to move from start to end or vice versa */
    UPROPERTY(EditAnywhere, Category="Movement")
    float CycleSpeed = 1.f;

    /** Will the bucket move back and forth between the start and end position infinitely */
    UPROPERTY(EditAnywhere, Category="Movement")
    bool bInfinite = true;

    /** If the bucket will stop briefly before continuing after a cycle */
    UPROPERTY(EditAnywhere, Category="Movement")
    float CycleDelay = 0.f;

    /* The position of the current cycle we are in */
    UPROPERTY(EditDefaultsOnly, Category="Movement", meta = (AllowPrivateAccess = "true"))
    float CyclePosition = 0.f;

    /** State of the movement 'which direction we are headed' */
    UPROPERTY(BlueprintReadOnly, Category="Movement")
    bool bReturnToStart = false;

#if WITH_EDITORONLY_DATA
    UPROPERTY()
    UBoxComponent* StartPreviewBox;

    UPROPERTY()
    UBoxComponent* EndPreviewBox;

    UPROPERTY(Transient)
    FVector LastStartPreviewLocation = FVector::ZeroVector;

    UPROPERTY(Transient)
    FVector LastEndPreviewLocation = FVector::ZeroVector;

    UPROPERTY(Transient)
    bool bPreviewBoxesInitialized = false;
#endif

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

#if WITH_EDITOR
    virtual void PostEditMove(bool bFinished) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    void UpdateEditorPreviewBoxes();
#endif
};
