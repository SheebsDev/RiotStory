#include "AimIndicatorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"

UAimIndicatorComponent::UAimIndicatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.f; //Every frame
	SetComponentTickEnabled(false);
}

void UAimIndicatorComponent::ShowAimIndicator()
{
	//Toggle Visibility and build initial projection path and start ticking the update
	RebuildProjectionPath();
	SetComponentTickEnabled(true);
}

void UAimIndicatorComponent::HideAimIndicator()
{
	//Toggle visibility and stop ticking the projection path update
	SetComponentTickEnabled(false);
	HidePathVisualization();
}

void UAimIndicatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Rebuild and update visuals
	RebuildProjectionPath();
}

void UAimIndicatorComponent::RebuildProjectionPath()
{
	ProjectionPathPoints.Reset();
	bProjectionPathHit = false;
	ProjectionPathHit = FHitResult();
	HidePathVisualization();

	AActor* const Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	const IAimIndicatorPathProvider* const PathProvider = Cast<IAimIndicatorPathProvider>(Owner);
	if (!PathProvider)
	{
		return;
	}

	FAimIndicatorPathData PathData;
	if (!PathProvider->GetAimIndicatorPathData(PathData))
	{
		return;
	}

	FPredictProjectilePathParams PredictParams;
	PredictParams.StartLocation = PathData.StartLocation;
	PredictParams.LaunchVelocity = PathData.LaunchVelocity;
	PredictParams.ProjectileRadius = FMath::Max(0.1f, PathData.ProjectileRadius);
	PredictParams.MaxSimTime = FMath::Max(PathData.MaxSimTime, KINDA_SMALL_NUMBER);
	PredictParams.SimFrequency = FMath::Max(1.0f, PathData.SimFrequency);
	PredictParams.TraceChannel = PathData.TraceChannel;
	PredictParams.bTraceWithCollision = PathData.bTraceWithCollision;
	PredictParams.bTraceWithChannel = true;
	PredictParams.bTraceComplex = PathData.bTraceComplex;

	// ignore the source and holder to avoid immediate self-collision in prediction
	PredictParams.ActorsToIgnore.Add(Owner);
	if (AActor* const Holder = Owner->GetOwner())
	{
		PredictParams.ActorsToIgnore.Add(Holder);
	}

	if (PathData.bOverrideGravityZ)
	{
		PredictParams.OverrideGravityZ = PathData.GravityZ;
	}

	FPredictProjectilePathResult PredictResult;
	UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);

	ProjectionPathPoints.Reserve(PredictResult.PathData.Num());
	for (const FPredictProjectilePathPointData& PointData : PredictResult.PathData)
	{
		ProjectionPathPoints.Add(PointData.Location);
	}

	if (ProjectionPathPoints.Num() == 0)
	{
		ProjectionPathPoints.Add(PathData.StartLocation);
	}

	bProjectionPathHit = PredictResult.HitResult.bBlockingHit;
	if (bProjectionPathHit)
	{
		ProjectionPathHit = PredictResult.HitResult;
	}

	UpdatePathVisualization();
}

void UAimIndicatorComponent::UpdatePathVisualization()
{
	if (!bUseSplineVisualization && !bDrawDebugFallback)
	{
		HidePathVisualization();
		return;
	}

	if (ProjectionPathPoints.Num() <= 0)
	{
		HidePathVisualization();
		return;
	}

	const int32 Step = FMath::Max(1, SplinePointStride);
	const int32 MaxPoints = FMath::Max(2, MaxSplinePoints);

	TArray<int32> SampledIndices;
	SampledIndices.Reserve(MaxPoints);

	for (int32 Index = 0; Index < ProjectionPathPoints.Num() && SampledIndices.Num() < MaxPoints; Index += Step)
	{
		SampledIndices.Add(Index);
	}

	if (SampledIndices.Num() == 0)
	{
		SampledIndices.Add(ProjectionPathPoints.Num() - 1);
	}

	const int32 LastPathPointIndex = ProjectionPathPoints.Num() - 1;
	if (SampledIndices.Last() != LastPathPointIndex)
	{
		if (SampledIndices.Num() < MaxPoints)
		{
			SampledIndices.Add(LastPathPointIndex);
		}
		else
		{
			SampledIndices.Last() = LastPathPointIndex;
		}
	}

	if (bUseSplineVisualization)
	{
		EnsureSplineComponent();

		if (IsValid(PathSplineComponent))
		{
			PathSplineComponent->ClearSplinePoints(false);
			for (const int32 PointIndex : SampledIndices)
			{
				PathSplineComponent->AddSplinePoint(ProjectionPathPoints[PointIndex], ESplineCoordinateSpace::World, false);
			}

			for (int32 PointIndex = 0; PointIndex < SampledIndices.Num(); ++PointIndex)
			{
				PathSplineComponent->SetSplinePointType(PointIndex, ESplinePointType::CurveClamped, false);
			}

			PathSplineComponent->UpdateSpline();
			PathSplineComponent->SetHiddenInGame(false);

			const int32 RequiredSegments = FMath::Max(0, SampledIndices.Num() - 1);
			EnsureSplineMeshPoolSize(RequiredSegments);

			for (int32 SegmentIndex = 0; SegmentIndex < SplineMeshPool.Num(); ++SegmentIndex)
			{
				USplineMeshComponent* const SplineMesh = SplineMeshPool[SegmentIndex];
				if (!IsValid(SplineMesh))
				{
					continue;
				}

				if (SegmentIndex >= RequiredSegments || !SplineSegmentMesh)
				{
					SplineMesh->SetHiddenInGame(true);
					continue;
				}

				SplineMesh->SetStaticMesh(SplineSegmentMesh);
				if (SplineSegmentMaterial)
				{
					SplineMesh->SetMaterial(0, SplineSegmentMaterial);
				}

				SplineMesh->SetForwardAxis(SplineForwardAxis, false);

				FVector StartLocation;
				FVector StartTangent;
				FVector EndLocation;
				FVector EndTangent;

				PathSplineComponent->GetLocationAndTangentAtSplinePoint(SegmentIndex, StartLocation, StartTangent, ESplineCoordinateSpace::Local);
				PathSplineComponent->GetLocationAndTangentAtSplinePoint(SegmentIndex + 1, EndLocation, EndTangent, ESplineCoordinateSpace::Local);

				StartTangent *= SplineTangentScale;
				EndTangent *= SplineTangentScale;

				SplineMesh->SetStartAndEnd(StartLocation, StartTangent, EndLocation, EndTangent, false);

				const float StartAlpha = RequiredSegments > 0 ? static_cast<float>(SegmentIndex) / static_cast<float>(RequiredSegments) : 0.0f;
				const float EndAlpha = RequiredSegments > 0 ? static_cast<float>(SegmentIndex + 1) / static_cast<float>(RequiredSegments) : 1.0f;

				SplineMesh->SetStartScale(FMath::Lerp(SplineStartScale, SplineEndScale, StartAlpha), false);
				SplineMesh->SetEndScale(FMath::Lerp(SplineStartScale, SplineEndScale, EndAlpha), true);
				SplineMesh->SetHiddenInGame(false);
			}
		}
	}
	else
	{
		HidePathVisualization();
	}

	if (bDrawDebugFallback && GetWorld())
	{
		for (int32 PointIdx = 0; PointIdx < SampledIndices.Num(); ++PointIdx)
		{
			const FVector PointLocation = ProjectionPathPoints[SampledIndices[PointIdx]];
			const float Radius = FMath::Lerp(2.0f, 6.0f, SampledIndices.Num() > 1 ? static_cast<float>(PointIdx) / static_cast<float>(SampledIndices.Num() - 1) : 1.0f);

			DrawDebugSphere(GetWorld(), PointLocation, Radius, 8, bProjectionPathHit ? FColor::Green : FColor::Yellow, false, 0.0f, 0, 1.25f);

			if (PointIdx > 0)
			{
				const FVector PrevPointLocation = ProjectionPathPoints[SampledIndices[PointIdx - 1]];
				DrawDebugLine(GetWorld(), PrevPointLocation, PointLocation, bProjectionPathHit ? FColor::Green : FColor::Yellow, false, 0.0f, 0, 1.0f);
			}
		}

		if (bProjectionPathHit)
		{
			DrawDebugSphere(GetWorld(), ProjectionPathHit.ImpactPoint, 8.0f, 10, FColor::Red, false, 0.0f, 0, 1.5f);
		}
	}
}

void UAimIndicatorComponent::HidePathVisualization()
{
	if (IsValid(PathSplineComponent))
	{
		PathSplineComponent->SetHiddenInGame(true);
	}

	for (USplineMeshComponent* const SplineMesh : SplineMeshPool)
	{
		if (IsValid(SplineMesh))
		{
			SplineMesh->SetHiddenInGame(true);
		}
	}
}

void UAimIndicatorComponent::EnsureSplineComponent()
{
	if (IsValid(PathSplineComponent))
	{
		return;
	}

	AActor* const Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	PathSplineComponent = NewObject<USplineComponent>(Owner, TEXT("AimPathSpline"));
	if (!IsValid(PathSplineComponent))
	{
		return;
	}

	if (USceneComponent* const OwnerRoot = Owner->GetRootComponent())
	{
		PathSplineComponent->SetupAttachment(OwnerRoot);
	}

	PathSplineComponent->SetMobility(EComponentMobility::Movable);
	PathSplineComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PathSplineComponent->SetGenerateOverlapEvents(false);
	PathSplineComponent->SetHiddenInGame(true);

	Owner->AddInstanceComponent(PathSplineComponent);
	PathSplineComponent->RegisterComponent();
}

void UAimIndicatorComponent::EnsureSplineMeshPoolSize(const int32 RequiredCount)
{
	if (RequiredCount <= SplineMeshPool.Num())
	{
		return;
	}

	for (int32 SegmentIndex = SplineMeshPool.Num(); SegmentIndex < RequiredCount; ++SegmentIndex)
	{
		const FName SegmentName(*FString::Printf(TEXT("AimPathSplineMesh_%d"), SegmentIndex));
		USplineMeshComponent* const NewSplineMesh = CreateSplineMeshComponent(SegmentName);

		if (!IsValid(NewSplineMesh))
		{
			return;
		}

		SplineMeshPool.Add(NewSplineMesh);
	}
}

USplineMeshComponent* UAimIndicatorComponent::CreateSplineMeshComponent(const FName Name) const
{
	AActor* const Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return nullptr;
	}

	USplineMeshComponent* const SplineMesh = NewObject<USplineMeshComponent>(Owner, Name);
	if (!IsValid(SplineMesh))
	{
		return nullptr;
	}

	if (IsValid(PathSplineComponent))
	{
		SplineMesh->SetupAttachment(PathSplineComponent);
	}
	else if (USceneComponent* const OwnerRoot = Owner->GetRootComponent())
	{
		SplineMesh->SetupAttachment(OwnerRoot);
	}

	SplineMesh->SetMobility(EComponentMobility::Movable);
	SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SplineMesh->SetGenerateOverlapEvents(false);
	SplineMesh->SetHiddenInGame(true);
	SplineMesh->SetForwardAxis(SplineForwardAxis, false);

	if (SplineSegmentMesh)
	{
		SplineMesh->SetStaticMesh(SplineSegmentMesh);
	}

	if (SplineSegmentMaterial)
	{
		SplineMesh->SetMaterial(0, SplineSegmentMaterial);
	}

	Owner->AddInstanceComponent(SplineMesh);
	SplineMesh->RegisterComponent();

	return SplineMesh;
}
