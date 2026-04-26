// Copyright Epic Games, Inc. All Rights Reserved.

#include "RiotStoryDebugUtil.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "RiotStory.h"

UWorld* FRiotStoryDebugUtil::ResolveWorld(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject) || GEngine == nullptr)
	{
		return nullptr;
	}

	return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
}

void FRiotStoryDebugUtil::PrintString(
	const UObject* WorldContextObject,
	const FString& Message,
	const bool bPrintToScreen,
	const bool bPrintToLog,
	const FLinearColor TextColor,
	const float Duration,
	const int32 Key)
{
	(void)WorldContextObject;

	if (bPrintToLog)
	{
		UE_LOG(LogRiotStory, Log, TEXT("%s"), *Message);
	}

	if (bPrintToScreen && GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(Key),
			Duration,
			TextColor.ToFColor(true),
			Message);
	}
}

void FRiotStoryDebugUtil::DrawDebugSphere(
	const UObject* WorldContextObject,
	const FVector Center,
	const float Radius,
	const FLinearColor Color,
	const float Duration,
	const float Thickness,
	const int32 Segments,
	const uint8 DepthPriority)
{
	UWorld* const World = ResolveWorld(WorldContextObject);
	if (!IsValid(World))
	{
		return;
	}

	::DrawDebugSphere(
		World,
		Center,
		Radius,
		Segments,
		Color.ToFColor(true),
		false,
		Duration,
		DepthPriority,
		Thickness);
}

void FRiotStoryDebugUtil::DrawDebugLine(
	const UObject* WorldContextObject,
	const FVector Start,
	const FVector End,
	const FLinearColor Color,
	const float Duration,
	const float Thickness,
	const uint8 DepthPriority)
{
	UWorld* const World = ResolveWorld(WorldContextObject);
	if (!IsValid(World))
	{
		return;
	}

	::DrawDebugLine(
		World,
		Start,
		End,
		Color.ToFColor(true),
		false,
		Duration,
		DepthPriority,
		Thickness);
}

void FRiotStoryDebugUtil::DrawDebugTrace(
	const UObject* WorldContextObject,
	const FVector TraceStart,
	const FVector TraceEnd,
	const bool bHit,
	const FVector HitLocation,
	const FLinearColor MissColor,
	const FLinearColor HitColor,
	const float Duration,
	const float Thickness,
	const float MarkerRadius,
	const int32 MarkerSegments,
	const uint8 DepthPriority)
{
	UWorld* const World = ResolveWorld(WorldContextObject);
	if (!IsValid(World))
	{
		return;
	}

	const FColor TraceColor = (bHit ? HitColor : MissColor).ToFColor(true);
	::DrawDebugLine(World, TraceStart, TraceEnd, TraceColor, false, Duration, DepthPriority, Thickness);

	const FVector MarkerLocation = bHit ? HitLocation : TraceEnd;
	::DrawDebugSphere(World, MarkerLocation, MarkerRadius, MarkerSegments, TraceColor, false, Duration, DepthPriority, Thickness);
}
