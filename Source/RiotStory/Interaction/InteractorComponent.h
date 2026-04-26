#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "Interaction/InteracteeTypes.h"
#include "InteractorComponent.generated.h"

class AActor;
class UInteracteeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInteractionResultSignature, AActor*, Interactor, AActor*, TargetActor, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FInteractableHighlightChangedSignature, AActor*, Interactor, UInteracteeComponent*, PreviousInteractee, UInteracteeComponent*, NewInteractee, AActor*, NewTargetActor, FInteractionDisplayInfo, NewDisplayInfo);

/**
 * Resolves and caches an interactable target from trace candidates every tick.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class RIOTSTORY_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UInteractorComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Max range used for interaction traces. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Trace", meta=(ClampMin=0.0, Units="cm"))
	float InteractionRange = 350.0f;

	/** Collision channel used to find interactable candidates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Trace")
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_GameTraceChannel2;

	/** Collision channel used to confirm line-of-sight to candidates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Trace")
	TEnumAsByte<ECollisionChannel> LineOfSightTraceChannel = ECC_Visibility;

	/** If true, interactions use complex collision when tracing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Trace")
	bool bTraceComplex = false;

	/** Draws a debug trace for interaction line traces. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug")
	bool bDrawDebugInteractionTrace = false;

	/** Also draws a sphere at each hit result from the multi-trace. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(EditCondition="bDrawDebugInteractionTrace"))
	bool bDrawDebugInteractionHitPoints = true;

	/** Color used when the interaction trace does not hit anything. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(EditCondition="bDrawDebugInteractionTrace"))
	FLinearColor DebugInteractionTraceMissColor = FLinearColor::Yellow;

	/** Color used when the interaction trace hits one or more results. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(EditCondition="bDrawDebugInteractionTrace"))
	FLinearColor DebugInteractionTraceHitColor = FLinearColor::Green;

	/** Lifetime for drawn interaction debug visuals. 0 means one-frame draw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(ClampMin=0.0, Units="s", EditCondition="bDrawDebugInteractionTrace"))
	float DebugInteractionTraceDuration = 0.0f;

	/** Line thickness for drawn interaction debug traces. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(ClampMin=0.0, EditCondition="bDrawDebugInteractionTrace"))
	float DebugInteractionTraceThickness = 1.5f;

	/** Radius of debug hit point markers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(ClampMin=0.0, Units="cm", EditCondition="bDrawDebugInteractionTrace"))
	float DebugInteractionHitPointRadius = 7.5f;

	/** Prints step-by-step highlight resolution debug messages. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug")
	bool bPrintHighlightDebug = false;

	/** Sends highlight debug messages to on-screen output. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(EditCondition="bPrintHighlightDebug"))
	bool bPrintHighlightDebugToScreen = true;

	/** Sends highlight debug messages to log output. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(EditCondition="bPrintHighlightDebug"))
	bool bPrintHighlightDebugToLog = true;

	/** Lifetime of each highlight debug print message. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(ClampMin=0.0, Units="s", EditCondition="bPrintHighlightDebug"))
	float HighlightDebugPrintDuration = 0.15f;

	/** Called whenever highlighted interactable changes. */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractableHighlightChangedSignature OnInteractableHighlightChanged;

	/** Called after each TryInteract attempt. */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInteractionResultSignature OnInteractionResult;

	/** Returns currently highlighted interactable actor, if any. */
	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetCurrentInteractableActor() const { return CurrentInteractableActor; }

	/** Returns currently highlighted interactee component, if any. */
	UFUNCTION(BlueprintPure, Category="Interaction")
	UInteracteeComponent* GetCurrentInteracteeComponent() const { return CurrentInteractee; }

	/** Returns current highlighted display info. */
	UFUNCTION(BlueprintPure, Category="Interaction")
	FInteractionDisplayInfo GetCurrentInteractableDisplayInfo() const { return CurrentInteractableDisplayInfo; }

	/** Attempts to interact with the currently highlighted target. */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	bool TryInteract();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction|Highlight", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInteracteeComponent> CurrentInteractee;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction|Highlight", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AActor> CurrentInteractableActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction|Highlight", meta=(AllowPrivateAccess="true"))
	FInteractionDisplayInfo CurrentInteractableDisplayInfo;

	/** Builds the trace start and end points from owner viewpoint and range. */
	bool BuildTraceStartAndEnd(FVector& OutStart, FVector& OutEnd) const;

	/** Finds the best visible interactable candidate from a forward trace. */
	bool FindInteractionTarget(UInteracteeComponent*& OutTargetInteractee, AActor*& OutTargetActor, FHitResult* OutHitResult = nullptr) const;

	/** Resolves a trace hit to an interactable actor or component. */
	bool ResolveInteractionTargetFromHit(const FHitResult& HitResult, UInteracteeComponent*& OutTargetInteractee, AActor*& OutTargetActor) const;

	/** Returns true if this candidate passes an LOS validation trace. */
	bool HasLineOfSightToCandidate(const FVector& TraceStart, const FHitResult& CandidateHit, UInteracteeComponent* CandidateInteractee, AActor* CandidateActor) const;

	/** Rebuilds and updates current highlighted interactable. */
	void RefreshHighlightedInteractable();

	/** Updates cached highlight and emits change event when target changed. */
	void UpdateHighlightedInteractable(UInteracteeComponent* NewTargetInteractee, AActor* NewTargetActor, const FInteractionDisplayInfo& NewDisplayInfo);

	/** Emits a keyed highlight debug message to screen/log when enabled. */
	void PrintHighlightDebugMessage(const FString& Message, int32 MessageKeyOffset) const;
};
