#include "Variant_Shooter/CardBucketSpawnPoint.h"

#include "Components/InterpToMovementComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ACardBucketSpawnPoint::ACardBucketSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* const RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(RootScene);

    EditorPreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EditorPreviewMesh"));
    if (EditorPreviewMesh)
    {
        EditorPreviewMesh->SetupAttachment(RootScene);
        EditorPreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        EditorPreviewMesh->SetGenerateOverlapEvents(false);
        EditorPreviewMesh->SetCanEverAffectNavigation(false);
        EditorPreviewMesh->SetHiddenInGame(true);
        EditorPreviewMesh->SetCastShadow(false);
        EditorPreviewMesh->bIsEditorOnly = true;
    }

    if (ControlPointOffsets.Num() == 0)
    {
        ControlPointOffsets.Add(FVector::ZeroVector);
        ControlPointOffsets.Add(FVector(150.f, 0.f, 0.f));
    }
}
