// Copyright Sheebs LoL. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Engine/EngineTypes.h"
#include "AimIndicatorPathProvider.generated.h"

USTRUCT(BlueprintType)
struct FAimIndicatorPathData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator")
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator")
	FVector LaunchVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator", meta = (ClampMin = 0.1, ClampMax = 1000.0, Units = "cm"))
	float ProjectileRadius = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator", meta = (ClampMin = 0.1, ClampMax = 10.0, Units = "s"))
	float MaxSimTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator", meta = (ClampMin = 1.0, ClampMax = 120.0))
	float SimFrequency = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator")
	bool bTraceWithCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator")
	bool bTraceComplex = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator")
	bool bOverrideGravityZ = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aim Indicator", meta = (Units = "cm/s^2"))
	float GravityZ = 0.0f;
};

UINTERFACE(MinimalAPI)
class UAimIndicatorPathProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Supplies projectile projection data to the aim indicator component.
 */
class RIOTSTORY_API IAimIndicatorPathProvider
{
	GENERATED_BODY()

public:

	/** Builds path prediction input data for the current shot setup. */
	virtual bool GetAimIndicatorPathData(FAimIndicatorPathData& OutPathData) const = 0;
};
