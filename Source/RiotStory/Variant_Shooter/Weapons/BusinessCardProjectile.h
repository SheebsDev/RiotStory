#pragma once

#include "CoreMinimal.h"
#include "BusinessCardProjectile.generated.h"

class UBoxComponent;
class UStaticMeshComponent; //Or plane?
class UProjectileMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCardProjectileRemovedDelegate, bool, bExpired);

UCLASS(Blueprintable)
class RIOTSTORY_API ABusinessCardProjectile : public AActor
{
    GENERATED_BODY()

    /** The visible mesh used for this projectile */
    UPROPERTY(VisibleAnywhere, Category ="Components", meta = (AllowPrivateAcces = "true"))
    UStaticMeshComponent* Mesh;

    /** Projectile Movement Component to handle "Card" throw physics for now */
    UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAcces = "true"))
    UProjectileMovementComponent* ProjectileMovement;
public:
    /** Constructor */
    ABusinessCardProjectile();

    /** How long to wait after a hit before destroying this projectile */
	UPROPERTY(EditAnywhere, Category="Projectile|Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float Lifetime = 8.0f;

    UFUNCTION(BlueprintCallable, Category="Projectile|Speed")
    void SetCardSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category="Projectile|Speed")
    void SetLaunchVelocity(const FVector& NewLaunchVelocity);

    /** Triggeres entering of the bucket and will destroy the object */
    UFUNCTION(BlueprintCallable, Category="Projectile|Destruction")
    void Consume();

    UPROPERTY(BlueprintAssignable, Category="Projectile")
    FCardProjectileRemovedDelegate OnCardProjectileRemoved;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	FTimerHandle LifetimeTimer;
    void OnLifetimeExpired();
};
