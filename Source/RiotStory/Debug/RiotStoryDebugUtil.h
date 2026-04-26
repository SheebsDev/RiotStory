// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
/** Lightweight debug helper wrappers for on-screen prints and trace visualization. */
class RIOTSTORY_API FRiotStoryDebugUtil
{
public:

	static void PrintString(
		const UObject* WorldContextObject,
		const FString& Message,
		bool bPrintToScreen = true,
		bool bPrintToLog = true,
		FLinearColor TextColor = FLinearColor(0.0f, 0.66f, 1.0f, 1.0f),
		float Duration = 2.0f,
		int32 Key = -1);

	static void DrawDebugSphere(
		const UObject* WorldContextObject,
		FVector Center,
		float Radius = 25.0f,
		FLinearColor Color = FLinearColor::Green,
		float Duration = 0.0f,
		float Thickness = 1.5f,
		int32 Segments = 12,
		uint8 DepthPriority = 0);

	static void DrawDebugLine(
		const UObject* WorldContextObject,
		FVector Start,
		FVector End,
		FLinearColor Color = FLinearColor::Yellow,
		float Duration = 0.0f,
		float Thickness = 1.5f,
		uint8 DepthPriority = 0);

	static void DrawDebugTrace(
		const UObject* WorldContextObject,
		FVector TraceStart,
		FVector TraceEnd,
		bool bHit = false,
		FVector HitLocation = FVector::ZeroVector,
		FLinearColor MissColor = FLinearColor::Yellow,
		FLinearColor HitColor = FLinearColor::Green,
		float Duration = 0.0f,
		float Thickness = 1.5f,
		float MarkerRadius = 8.0f,
		int32 MarkerSegments = 12,
		uint8 DepthPriority = 0);

private:

	static UWorld* ResolveWorld(const UObject* WorldContextObject);
};
