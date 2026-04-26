#include "CardBucket.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

#include "Variant_Shooter/Weapons/BusinessCardProjectile.h"

ACardBucket::ACardBucket()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
    SetRootComponent(Mesh);

    BoxCollision = CreateDefaultSubobject<UBoxComponent>("Box Collision");
    BoxCollision->SetupAttachment(Mesh);
    BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ACardBucket::OnBoxOverlap);

#if WITH_EDITORONLY_DATA
    StartPreviewBox = CreateEditorOnlyDefaultSubobject<UBoxComponent>(TEXT("Start Preview Box"));
    if (StartPreviewBox)
    {
        StartPreviewBox->SetupAttachment(RootComponent);
        StartPreviewBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        StartPreviewBox->SetGenerateOverlapEvents(false);
        StartPreviewBox->SetCanEverAffectNavigation(false);
        StartPreviewBox->SetMobility(EComponentMobility::Movable);
        StartPreviewBox->SetHiddenInGame(true);
        StartPreviewBox->bIsEditorOnly = true;
        StartPreviewBox->ShapeColor = FColor(64, 160, 255);
        StartPreviewBox->SetUsingAbsoluteLocation(true);
        StartPreviewBox->SetUsingAbsoluteRotation(true);
        StartPreviewBox->SetUsingAbsoluteScale(true);
        StartPreviewBox->bDrawOnlyIfSelected = true;
    }

    EndPreviewBox = CreateEditorOnlyDefaultSubobject<UBoxComponent>(TEXT("End Preview Box"));
    if (EndPreviewBox)
    {
        EndPreviewBox->SetupAttachment(RootComponent);
        EndPreviewBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        EndPreviewBox->SetGenerateOverlapEvents(false);
        EndPreviewBox->SetCanEverAffectNavigation(false);
        EndPreviewBox->SetMobility(EComponentMobility::Movable);
        EndPreviewBox->SetHiddenInGame(true);
        EndPreviewBox->bIsEditorOnly = true;
        EndPreviewBox->ShapeColor = FColor(255, 180, 64);
        EndPreviewBox->SetUsingAbsoluteLocation(true);
        EndPreviewBox->SetUsingAbsoluteRotation(true);
        EndPreviewBox->SetUsingAbsoluteScale(true);
        EndPreviewBox->bDrawOnlyIfSelected = true;
    }
#endif

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    SetActorTickEnabled(false);
}

void ACardBucket::Tick(float DeltaSeconds)
{
    //Interp back and forther between the two points
    float CycleMovement = CycleSpeed * DeltaSeconds;

    if (bReturnToStart)
    {
        CyclePosition -= CycleMovement;
    }
    else
    {
        CyclePosition += CycleMovement;
    }

    //If we've gone out of bounds let's clamp and reverse
    if (CyclePosition >= 1.f || CyclePosition <= 0.f)
    {
        bReturnToStart = !bReturnToStart;
        CyclePosition = FMath::Clamp(CyclePosition, 0.f, 1.f);

        //We've reached our destination let's stop
        if (!bInfinite)
        {
            SetActorTickEnabled(false);
        }
    }

    FVector NewLocation = FMath::Lerp(StartPosition, EndPosition, CyclePosition);
    SetActorLocation(NewLocation);
}

void ACardBucket::BeginMoving()
{
    SetActorTickEnabled(true);
}

void ACardBucket::StopMoving()
{
    SetActorTickEnabled(false);
}

void ACardBucket::BeginPlay()
{
    Super::BeginPlay();
}

void ACardBucket::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

#if WITH_EDITOR
    UpdateEditorPreviewBoxes();
#endif
}

#if WITH_EDITOR
void ACardBucket::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);
    UpdateEditorPreviewBoxes();
}

void ACardBucket::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    UpdateEditorPreviewBoxes();
}
#endif

void ACardBucket::OnBoxOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    if (OtherActor && OtherActor->IsA(ABusinessCardProjectile::StaticClass()))
    {
        
    }
}

#if WITH_EDITOR
void ACardBucket::UpdateEditorPreviewBoxes()
{
#if WITH_EDITORONLY_DATA
    if (!StartPreviewBox || !EndPreviewBox)
    {
        return;
    }

    const FVector CurrentStartLocation = StartPreviewBox->GetComponentLocation();
    const FVector CurrentEndLocation = EndPreviewBox->GetComponentLocation();

    if (bPreviewBoxesInitialized)
    {
        if (!CurrentStartLocation.Equals(LastStartPreviewLocation, 0.1f))
        {
            StartPosition = CurrentStartLocation;
        }

        if (!CurrentEndLocation.Equals(LastEndPreviewLocation, 0.1f))
        {
            EndPosition = CurrentEndLocation;
        }
    }

    FVector PreviewExtent(25.f);
    if (Mesh)
    {
        const UStaticMesh* StaticMesh = Mesh->GetStaticMesh();
        if (StaticMesh)
        {
            PreviewExtent = StaticMesh->GetBounds().BoxExtent * Mesh->GetRelativeScale3D().GetAbs();
            PreviewExtent.X = FMath::Max(PreviewExtent.X, 1.f);
            PreviewExtent.Y = FMath::Max(PreviewExtent.Y, 1.f);
            PreviewExtent.Z = FMath::Max(PreviewExtent.Z, 1.f);
        }
    }

    StartPreviewBox->SetBoxExtent(PreviewExtent);
    EndPreviewBox->SetBoxExtent(PreviewExtent);

    const FRotator PreviewRotation = GetActorRotation();
    StartPreviewBox->SetWorldLocationAndRotation(StartPosition, PreviewRotation);
    EndPreviewBox->SetWorldLocationAndRotation(EndPosition, PreviewRotation);

    LastStartPreviewLocation = StartPosition;
    LastEndPreviewLocation = EndPosition;
    bPreviewBoxesInitialized = true;
#endif
}
#endif
