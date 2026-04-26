// Copyright Sheebs LoL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SplineMeshComponent.h"
#include "AimIndicatorPathProvider.h"
#include "AimIndicatorComponent.generated.h"

class USplineComponent;
class USplineMeshComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Weapons), meta=(BlueprintSpawnableComponent))
class RIOTSTORY_API UAimIndicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Constructor */
	UAimIndicatorComponent();

	/** Start displaying the aim indicator */
	UFUNCTION(BlueprintCallable, Category="Aim Indicator")
	void ShowAimIndicator();

	/** Stop displaying the aim indicator */
	UFUNCTION(BlueprintCallable, Category="Aim Indicator")
	void HideAimIndicator();

	/** Last generated path points from the projectile prediction call */
	UPROPERTY(BlueprintReadOnly, Category="Aim Indicator")
	TArray<FVector> ProjectionPathPoints;

	/** Whether the last projection hit blocking geometry */
	UPROPERTY(BlueprintReadOnly, Category="Aim Indicator")
	bool bProjectionPathHit = false;

	/** Last hit result from the projection path */
	UPROPERTY(BlueprintReadOnly, Category="Aim Indicator")
	FHitResult ProjectionPathHit;

	/** Enable spline-based path visualization */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization")
	bool bUseSplineVisualization = true;

	/** Static mesh used for each spline segment */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization")
	TObjectPtr<UStaticMesh> SplineSegmentMesh;

	/** Optional material override for each spline segment */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization")
	TObjectPtr<UMaterialInterface> SplineSegmentMaterial;

	/** Scale applied to the first spline mesh segment point */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization", meta = (ClampMin = 0.001, ClampMax = 5.0))
	FVector2D SplineStartScale = FVector2D(0.05f, 0.05f);

	/** Scale applied to the last spline mesh segment point */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization", meta = (ClampMin = 0.001, ClampMax = 5.0))
	FVector2D SplineEndScale = FVector2D(0.08f, 0.08f);

	/** Tangent multiplier to smooth the spline mesh curvature */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization", meta = (ClampMin = 0.0, ClampMax = 5.0))
	float SplineTangentScale = 0.35f;

	/** Forward axis expected by the segment mesh */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization")
	TEnumAsByte<ESplineMeshAxis::Type> SplineForwardAxis = ESplineMeshAxis::X;

	/** Maximum number of sampled points used to build the spline */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization", meta = (ClampMin = 2, ClampMax = 256))
	int32 MaxSplinePoints = 48;

	/** Uses every Nth projected point when building the spline */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization", meta = (ClampMin = 1, ClampMax = 20))
	int32 SplinePointStride = 1;

	/** Draw debug points and line segments for one-frame fallback visuals */
	UPROPERTY(EditAnywhere, Category="Aim Indicator|Visualization")
	bool bDrawDebugFallback = true;

protected:
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void RebuildProjectionPath();
	void UpdatePathVisualization();
	void HidePathVisualization();
	void EnsureSplineComponent();
	void EnsureSplineMeshPoolSize(int32 RequiredCount);
	USplineMeshComponent* CreateSplineMeshComponent(FName Name) const;

	UPROPERTY(Transient)
	TObjectPtr<USplineComponent> PathSplineComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USplineMeshComponent>> SplineMeshPool;
};
