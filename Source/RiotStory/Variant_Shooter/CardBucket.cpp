#include "CardBucket.h"

#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InterpToMovementComponent.h"

#include "Variant_Shooter/Weapons/BusinessCardProjectile.h"

ACardBucket::ACardBucket()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
    SetRootComponent(Mesh);

    BoxCollision = CreateDefaultSubobject<UBoxComponent>("Box Collision");
    BoxCollision->SetupAttachment(Mesh);
    BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ACardBucket::OnBoxOverlap);

    InterpToMovement = CreateDefaultSubobject<UInterpToMovementComponent>(TEXT("InterpToMovement"));
    InterpToMovement->SetUpdatedComponent(RootComponent);
    InterpToMovement->bAutoActivate = false;

    ControlPointOffsets = {
        FVector::ZeroVector,
        FVector(150.f, 0.f, 0.f)
    };

    PrimaryActorTick.bCanEverTick = false;
}

void ACardBucket::BeginMoving()
{
    if (CycleSpeed <= KINDA_SMALL_NUMBER)
    {
        StopMoving();
        return;
    }

    ApplyMovementSettings();
    RebuildMovementControlPoints();

    if (InterpToMovement && ControlPointOffsets.Num() >= 2)
    {
        InterpToMovement->RestartMovement(1.f);
        InterpToMovement->Activate(true);
    }
}

void ACardBucket::StopMoving()
{
    if (InterpToMovement)
    {
        InterpToMovement->StopMovementImmediately();
        InterpToMovement->Deactivate();
    }
}

void ACardBucket::BeginPlay()
{
    Super::BeginPlay();

    ApplyMovementSettings();
}

void ACardBucket::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyMovementSettings();
}

void ACardBucket::SetControlPointOffsets(const TArray<FVector>& NewPoints)
{
    ControlPointOffsets = NewPoints;
    RebuildMovementControlPoints();
}

void ACardBucket::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ABusinessCardProjectile::StaticClass()))
    {
    }
}

void ACardBucket::ApplyMovementSettings()
{
    InterpToMovement->Duration = CycleSpeed;
    InterpToMovement->BehaviourType = bInfinite ? EInterpToBehaviourType::PingPong : EInterpToBehaviourType::OneShot;
}

void ACardBucket::RebuildMovementControlPoints()
{
    if (!InterpToMovement || ControlPointOffsets.Num() < 2)
    {
        return;
    }

    InterpToMovement->StopMovementImmediately();
    InterpToMovement->ResetControlPoints();

    FVector UpdatedComponentScale = FVector::OneVector;
    if (USceneComponent* const UpdatedComponent = InterpToMovement->UpdatedComponent.Get())
    {
        UpdatedComponentScale = UpdatedComponent->GetComponentScale();
    }

    UpdatedComponentScale.X = FMath::Abs(UpdatedComponentScale.X);
    UpdatedComponentScale.Y = FMath::Abs(UpdatedComponentScale.Y);
    UpdatedComponentScale.Z = FMath::Abs(UpdatedComponentScale.Z);

    for (const FVector& Offset : ControlPointOffsets)
    {
        const FVector ScaledOffset = Offset * UpdatedComponentScale;
        InterpToMovement->AddControlPointPosition(ScaledOffset, true);
    }

    InterpToMovement->FinaliseControlPoints();
}
