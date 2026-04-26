#include "Interaction/InteractorComponent.h"
#include "Interaction/InteracteeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Debug/RiotStoryDebugUtil.h"

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(true);
}

void UInteractorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RefreshHighlightedInteractable();
}

bool UInteractorComponent::TryInteract()
{
	AActor* const Interactor = GetOwner();
	if (!IsValid(Interactor))
	{
		OnInteractionResult.Broadcast(nullptr, nullptr, false);
		return false;
	}

	if (!IsValid(CurrentInteractee.Get()) || !IsValid(CurrentInteractableActor.Get()))
	{
		RefreshHighlightedInteractable();
	}

	UInteracteeComponent* const TargetInteractee = CurrentInteractee.Get();
	AActor* const TargetActor = CurrentInteractableActor.Get();
	if (!IsValid(TargetInteractee) || !IsValid(TargetActor))
	{
		OnInteractionResult.Broadcast(Interactor, nullptr, false);
		return false;
	}

	if (!TargetInteractee->CanInteract(Interactor))
	{
		RefreshHighlightedInteractable();
		OnInteractionResult.Broadcast(Interactor, TargetActor, false);
		return false;
	}

	const bool bWasSuccessful = TargetInteractee->ExecuteInteraction(Interactor);
	OnInteractionResult.Broadcast(Interactor, TargetActor, bWasSuccessful);

	RefreshHighlightedInteractable();
	return bWasSuccessful;
}

bool UInteractorComponent::BuildTraceStartAndEnd(FVector& OutStart, FVector& OutEnd) const
{
	const AActor* const OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return false;
	}

	FRotator ViewRotation = OwnerActor->GetActorRotation();
	if (const APawn* const OwnerPawn = Cast<APawn>(OwnerActor))
	{
		OwnerPawn->GetActorEyesViewPoint(OutStart, ViewRotation);
	}
	else
	{
		OutStart = OwnerActor->GetActorLocation();
	}

	OutEnd = OutStart + (ViewRotation.Vector() * InteractionRange);
	return true;
}

bool UInteractorComponent::FindInteractionTarget(UInteracteeComponent*& OutTargetInteractee, AActor*& OutTargetActor, FHitResult* OutHitResult) const
{
	OutTargetInteractee = nullptr;
	OutTargetActor = nullptr;

	const UWorld* const World = GetWorld();
	AActor* const OwnerActor = GetOwner();
	if (!IsValid(World) || !IsValid(OwnerActor))
	{
		return false;
	}

	FVector TraceStart = FVector::ZeroVector;
	FVector TraceEnd = FVector::ZeroVector;
	if (!BuildTraceStartAndEnd(TraceStart, TraceEnd))
	{
		return false;
	}

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(InteractorTrace), bTraceComplex);
	QueryParams.AddIgnoredActor(OwnerActor);

	const bool bDidHit = World->LineTraceMultiByChannel(HitResults, TraceStart, TraceEnd, InteractionTraceChannel, QueryParams);
	if (bDrawDebugInteractionTrace)
	{
		FVector FirstHitLocation = TraceEnd;
		bool bHasBlockingHit = false;
		for (const FHitResult& HitResult : HitResults)
		{
			if (HitResult.bBlockingHit)
			{
				FirstHitLocation = HitResult.ImpactPoint;
				bHasBlockingHit = true;
				break;
			}
		}

		FRiotStoryDebugUtil::DrawDebugTrace(
			GetWorld(),
			TraceStart,
			TraceEnd,
			bHasBlockingHit,
			FirstHitLocation,
			DebugInteractionTraceMissColor,
			DebugInteractionTraceHitColor,
			DebugInteractionTraceDuration,
			DebugInteractionTraceThickness,
			DebugInteractionHitPointRadius);

		if (bDrawDebugInteractionHitPoints)
		{
			for (const FHitResult& HitResult : HitResults)
			{
				FRiotStoryDebugUtil::DrawDebugSphere(
					GetWorld(),
					HitResult.ImpactPoint,
					DebugInteractionHitPointRadius,
					DebugInteractionTraceHitColor,
					DebugInteractionTraceDuration,
					DebugInteractionTraceThickness);
			}
		}
	}

	if (!bDidHit || HitResults.Num() == 0)
	{
		return false;
	}

	HitResults.Sort([](const FHitResult& A, const FHitResult& B)
	{
		return A.Distance < B.Distance;
	});

	for (const FHitResult& CandidateHit : HitResults)
	{
		UInteracteeComponent* CandidateInteractee = nullptr;
		AActor* CandidateActor = nullptr;
		if (!ResolveInteractionTargetFromHit(CandidateHit, CandidateInteractee, CandidateActor))
		{
			continue;
		}

		if (!CandidateInteractee->CanInteract(OwnerActor))
		{
			continue;
		}

		if (!HasLineOfSightToCandidate(TraceStart, CandidateHit, CandidateInteractee, CandidateActor))
		{
			continue;
		}

		OutTargetInteractee = CandidateInteractee;
		OutTargetActor = CandidateActor;

		if (OutHitResult != nullptr)
		{
			*OutHitResult = CandidateHit;
		}

		return true;
	}

	return false;
}

bool UInteractorComponent::ResolveInteractionTargetFromHit(const FHitResult& HitResult, UInteracteeComponent*& OutTargetInteractee, AActor*& OutTargetActor) const
{
	OutTargetInteractee = nullptr;
	OutTargetActor = nullptr;

	AActor* const HitActor = HitResult.GetActor();
	if (!IsValid(HitActor))
	{
		PrintHighlightDebugMessage(TEXT("ResolveHit: invalid hit actor"), 10);
		return false;
	}

	UPrimitiveComponent* const HitComponent = HitResult.GetComponent();

	TInlineComponentArray<UInteracteeComponent*> Interactees;
	HitActor->GetComponents(Interactees);
	if (Interactees.Num() == 0)
	{
		PrintHighlightDebugMessage(FString::Printf(TEXT("ResolveHit: actor '%s' has no interactees"), *GetNameSafe(HitActor)), 11);
		return false;
	}

	// Prefer interactees explicitly bound to the hit primitive.
	if (IsValid(HitComponent))
	{
		PrintHighlightDebugMessage(FString::Printf(TEXT("ResolveHit: checking component '%s' on actor '%s' (%d interactees)"), *GetNameSafe(HitComponent), *GetNameSafe(HitActor), Interactees.Num()), 12);
		for (UInteracteeComponent* const Interactee : Interactees)
		{
			if (!IsValid(Interactee) || !Interactee->IsInteractionEnabled())
			{
				PrintHighlightDebugMessage(FString::Printf(TEXT("ResolveHit: skip interactee '%s' (valid=%d active=%d enabled=%d)"), *GetNameSafe(Interactee), IsValid(Interactee) ? 1 : 0, IsValid(Interactee) && Interactee->IsActive() ? 1 : 0, IsValid(Interactee) && Interactee->IsInteractionEnabled() ? 1 : 0), 13);
				continue;
			}

			if (Interactee->MatchesCollisionComponent(HitComponent))
			{
				OutTargetInteractee = Interactee;
				OutTargetActor = HitActor;
				PrintHighlightDebugMessage(FString::Printf(TEXT("ResolveHit: matched interactee '%s' to hit component '%s'"), *GetNameSafe(Interactee), *GetNameSafe(HitComponent)), 14);
				return true;
			}
		}
	}

	// Fallback to first active interactee with valid interaction collision binding.
	for (UInteracteeComponent* const Interactee : Interactees)
	{
		if (!IsValid(Interactee) || !Interactee->IsInteractionEnabled())
		{
			continue;
		}

		OutTargetInteractee = Interactee;
		OutTargetActor = HitActor;
		PrintHighlightDebugMessage(FString::Printf(TEXT("ResolveHit: fallback interactee '%s' on actor '%s'"), *GetNameSafe(Interactee), *GetNameSafe(HitActor)), 15);
		return true;
	}

	PrintHighlightDebugMessage(FString::Printf(TEXT("ResolveHit: no valid interactee on actor '%s'"), *GetNameSafe(HitActor)), 16);
	return false;
}

bool UInteractorComponent::HasLineOfSightToCandidate(const FVector& TraceStart, const FHitResult& CandidateHit, UInteracteeComponent* CandidateInteractee, AActor* CandidateActor) const
{
	const UWorld* const World = GetWorld();
	AActor* const OwnerActor = GetOwner();
	if (!IsValid(World) || !IsValid(OwnerActor) || !IsValid(CandidateActor) || !IsValid(CandidateInteractee))
	{
		return false;
	}

	FVector TargetPoint = CandidateHit.ImpactPoint;
	if (TargetPoint.IsNearlyZero())
	{
		TargetPoint = CandidateActor->GetActorLocation();
	}

	FHitResult LOSHit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(InteractorLOS), bTraceComplex);
	QueryParams.AddIgnoredActor(OwnerActor);

	const bool bBlocked = World->LineTraceSingleByChannel(LOSHit, TraceStart, TargetPoint, LineOfSightTraceChannel, QueryParams);
	if (!bBlocked)
	{
		return true;
	}

	if (LOSHit.GetActor() == CandidateActor)
	{
		return true;
	}

	if (UPrimitiveComponent* const CandidateCollision = CandidateInteractee->GetInteractionCollisionComponent())
	{
		return LOSHit.GetComponent() == CandidateCollision;
	}

	return false;
}

void UInteractorComponent::RefreshHighlightedInteractable()
{
	UInteracteeComponent* NewTargetInteractee = nullptr;
	AActor* NewTargetActor = nullptr;
	FInteractionDisplayInfo NewDisplayInfo;

	if (FindInteractionTarget(NewTargetInteractee, NewTargetActor))
	{
		PrintHighlightDebugMessage(FString::Printf(TEXT("Highlight: found target actor='%s' interactee='%s'"), *GetNameSafe(NewTargetActor), *GetNameSafe(NewTargetInteractee)), 1);
		if (IsValid(NewTargetInteractee))
		{
			NewTargetInteractee->GetInteractionDisplayInfo(GetOwner(), NewDisplayInfo);
		}
	}
	else
	{
		PrintHighlightDebugMessage(TEXT("Highlight: no valid target this tick"), 2);
	}

	UpdateHighlightedInteractable(NewTargetInteractee, NewTargetActor, NewDisplayInfo);
}

void UInteractorComponent::UpdateHighlightedInteractable(UInteracteeComponent* NewTargetInteractee, AActor* NewTargetActor, const FInteractionDisplayInfo& NewDisplayInfo)
{
	if (CurrentInteractee.Get() == NewTargetInteractee && CurrentInteractableActor.Get() == NewTargetActor)
	{
		PrintHighlightDebugMessage(TEXT("Highlight: unchanged"), 3);
		return;
	}

	UInteracteeComponent* const PreviousInteractee = CurrentInteractee.Get();
	
	AActor* const Interactor = GetOwner();

	CurrentInteractee = NewTargetInteractee;
	CurrentInteractableActor = NewTargetActor;
	CurrentInteractableDisplayInfo = IsValid(NewTargetInteractee) ? NewDisplayInfo : FInteractionDisplayInfo();
	PrintHighlightDebugMessage(
		FString::Printf(
			TEXT("Highlight: changed prev='%s' new='%s' actor='%s'"),
			*GetNameSafe(PreviousInteractee),
			*GetNameSafe(CurrentInteractee.Get()),
			*GetNameSafe(CurrentInteractableActor.Get())),
		4);

	if (PreviousInteractee)
	{
		PreviousInteractee->ToggleHighlight(false);
	}

	if (IsValid(CurrentInteractee.Get()))
	{
		CurrentInteractee->ToggleHighlight(true);
	}
	
	OnInteractableHighlightChanged.Broadcast(Interactor, PreviousInteractee, CurrentInteractee.Get(), CurrentInteractableActor.Get(), CurrentInteractableDisplayInfo);
}

void UInteractorComponent::PrintHighlightDebugMessage(const FString& Message, const int32 MessageKeyOffset) const
{
	if (!bPrintHighlightDebug)
	{
		return;
	}

	constexpr int32 HighlightDebugMessageKeyBase = 52000;
	/*FRiotStoryDebugUtil::PrintString(
		GetWorld(),
		Message,
		bPrintHighlightDebugToScreen,
		bPrintHighlightDebugToLog,
		FLinearColor(0.1f, 0.75f, 1.0f, 1.0f),
		HighlightDebugPrintDuration,
		HighlightDebugMessageKeyBase + MessageKeyOffset);*/
}
