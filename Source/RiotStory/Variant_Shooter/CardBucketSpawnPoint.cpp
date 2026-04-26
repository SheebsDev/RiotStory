#include "Variant_Shooter/CardBucketSpawnPoint.h"

#include "Components/SceneComponent.h"

ACardBucketSpawnPoint::ACardBucketSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* const RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(RootScene);
}

