#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardBucketSpawnPoint.generated.h"

class ACardBucket;
class USceneComponent;
class UInterpToMovementComponent;
class UStaticMeshComponent;

UCLASS()
class RIOTSTORY_API ACardBucketSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    ACardBucketSpawnPoint();

    /** Stable id used by bucket spawn table rows to target this point. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket")
    FName SpawnPointId = NAME_None;

    /** Bucket class to spawn from this point */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket")
    TSubclassOf<ACardBucket> BucketClass;

    /** Local control-point offsets used for viewport editing and mirrored into InterpToMovement. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement", meta=(MakeEditWidget=true))
    TArray<FVector> ControlPointOffsets;

    /** Editor-only mesh for visualizing the spawn point in the level. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Card Bucket|Editor", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UStaticMeshComponent> EditorPreviewMesh;

    /** Override speed for bucket cycle movement. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    float CycleSpeed = 1.f;

    /** Override infinite back-and-forth behavior. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    bool bInfinite = true;

    /** Optional per-point start moving behavior. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Bucket|Movement")
    bool bBeginMoving = true;
};
