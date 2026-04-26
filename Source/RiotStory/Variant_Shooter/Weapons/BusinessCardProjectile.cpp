#include "BusinessCardProjectile.h"
#include "Components\BoxComponent.h"
#include "Components\StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"
#include "EngineUtils.h"

ABusinessCardProjectile::ABusinessCardProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
    ProjectileMovement->UpdatedComponent = Mesh;

	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bShouldBounce = true;
}

void ABusinessCardProjectile::SetCardSpeed(float NewSpeed)
{
    ProjectileMovement->InitialSpeed = NewSpeed;
    ProjectileMovement->Velocity = GetActorForwardVector() * NewSpeed;
}

void ABusinessCardProjectile::SetLaunchVelocity(const FVector& NewLaunchVelocity)
{
    ProjectileMovement->Velocity = NewLaunchVelocity;
    ProjectileMovement->InitialSpeed = NewLaunchVelocity.Size();
}

void ABusinessCardProjectile::Consume()
{
    OnCardProjectileRemoved.Broadcast(false);
    Destroy();
}

void ABusinessCardProjectile::BeginPlay()
{
    Super::BeginPlay();

    // ignore the pawn that shot this projectile
	Mesh->IgnoreActorWhenMoving(GetInstigator(), true);

	// ensure cards do not collide with other cards
	for (TActorIterator<ABusinessCardProjectile> CardIt(GetWorld()); CardIt; ++CardIt)
	{
		ABusinessCardProjectile* OtherCard = *CardIt;
		if (!OtherCard || OtherCard == this)
		{
			continue;
		}

		Mesh->IgnoreActorWhenMoving(OtherCard, true);
		OtherCard->Mesh->IgnoreActorWhenMoving(this, true);
	}

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(LifetimeTimer, this, &ABusinessCardProjectile::OnLifetimeExpired, Lifetime, false);
    }
}

void ABusinessCardProjectile::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(LifetimeTimer);
    }
}

void ABusinessCardProjectile::OnLifetimeExpired()
{
    OnCardProjectileRemoved.Broadcast(true);
    Destroy();
}
