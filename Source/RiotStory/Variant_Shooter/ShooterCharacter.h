// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RiotStoryCharacter.h"
#include "ShooterWeaponHolder.h"
#include "Interaction/InventoryItemConsumerInterface.h"
#include "Variant_Shooter/Weapons/AimIndicatorPathProvider.h"
#include "ShooterCharacter.generated.h"

class AShooterWeapon;
class UInputAction;
class UInputComponent;
class UPawnNoiseEmitterComponent;
class UAimIndicatorComponent;
class ABusinessCardProjectile;
class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamagedDelegate, float, LifePercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAvailableCardCountChangedDelegate, int32, NewCount);

/**
 *  A player controllable first person shooter character
 *  Manages a weapon inventory through the IShooterWeaponHolder interface
 *  Manages health and death
 */
UCLASS(abstract)
class RIOTSTORY_API AShooterCharacter : public ARiotStoryCharacter, public IShooterWeaponHolder, public IAimIndicatorPathProvider, public IInventoryItemConsumerInterface
{
	GENERATED_BODY()
	
	/** AI Noise emitter component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPawnNoiseEmitterComponent* PawnNoiseEmitter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAimIndicatorComponent* AimIndicator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* Inventory;

protected:

	UPROPERTY(EditAnywhere, Category="Cards")
	TSubclassOf<ABusinessCardProjectile> CardProjectileClass;

	UPROPERTY(EditAnywhere, Category="Cards")
	float CardSpeed = 100.f;

	UPROPERTY(EditAnywhere, Category="Cards")
	float SpeedGain = 20.f;

	UPROPERTY(EditAnywhere, Category="Cards")
	float MaxCardSpeed = 2000.f;

	UPROPERTY(EditAnywhere, Category="Cards")
	FVector ThrowLocation;

	/** Fire weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* FireAction;

	/** Zoom aim input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* ZoomAction;

	/** Switch weapon input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SwitchWeaponAction;

	/** Name of the first person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** Name of the third person mesh weapon socket */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** Max distance to use for aim traces */
	UPROPERTY(EditAnywhere, Category ="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	/** Max HP this character can have */
	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHP = 500.0f;

	/** Current HP remaining to this character */
	float CurrentHP = 0.0f;

	/** Team ID for this character*/
	UPROPERTY(EditAnywhere, Category="Team")
	uint8 TeamByte = 0;

	/** Actor tag to grant this character when it dies */
	UPROPERTY(EditAnywhere, Category="Team")
	FName DeathTag = FName("Dead");

	/** List of weapons picked up by the character */
	TArray<AShooterWeapon*> OwnedWeapons;

	/** Weapon currently equipped and ready to shoot with */
	TObjectPtr<AShooterWeapon> CurrentWeapon;

	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	FTimerHandle RespawnTimer;

public:

	FBulletCountUpdatedDelegate OnBulletCountUpdated;
	FDamagedDelegate OnDamaged;

	UPROPERTY(BlueprintAssignable, Category="Cards")
	FAvailableCardCountChangedDelegate OnAvailableCardCountChanged;

public:

	/** Constructor */
	AShooterCharacter();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Cards")
	void SetEnableCardThrowing(bool bEnable) { bIsCardThrowingEnabled = bEnable; }

	/** Use this setter to update the number of cards over changing the private var NumCardsAvailable */
	UFUNCTION(BlueprintCallable, Category="Cards")
	void SetAvailableNumberOfCards(int32 Value);

	UFUNCTION(BlueprintCallable, Category="Cards")
	int32 GetAvailableNumberOfCards() { return NumCardsAvailable; }
	
	UFUNCTION(BlueprintCallable, Category="Aim")
	UAimIndicatorComponent* GetAimIndicator() { return AimIndicator; }

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	/** Handle incoming damage */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:

	/** Handles aim inputs from either controls or UI interfaces */
	virtual void DoAim(float Yaw, float Pitch) override;

	/** Handles zoom aim input from either controls or UI interfaces */
	virtual void DoStartZoom();

	/** Handles zoom end input from either controls or UI interfaces */
	virtual void DoStopZoom();

	/** Handles move inputs from either controls or UI interfaces */
	virtual void DoMove(float Right, float Forward)  override;

	/** Handles jump start inputs from either controls or UI interfaces */
	virtual void DoJumpStart()  override;

	/** Handles jump end inputs from either controls or UI interfaces */
	virtual void DoJumpEnd()  override;

	/** Handles start firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartFiring();

	/** Handles stop firing input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopFiring();

	/** Handles switch weapon input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSwitchWeapon();

public:

	//~Begin IShooterWeaponHolder interface

	/** Attaches a weapon's meshes to the owner */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** Plays the firing montage for the weapon */
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	/** Applies weapon recoil to the owner */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** Updates the weapon's HUD with the current ammo count */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** Calculates and returns the aim location for the weapon */
	virtual FVector GetWeaponTargetLocation() override;

	/** Gives a weapon of this class to the owner */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass) override;

	/** Activates the passed weapon */
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;

	/** Deactivates the passed weapon */
	virtual void OnWeaponDeactivated(AShooterWeapon* Weapon) override;

	/** Notifies the owner that the weapon cooldown has expired and it's ready to shoot again */
	virtual void OnSemiWeaponRefire() override;

	//~End IShooterWeaponHolder interface

	//~Begin IInventoryItemConsumerInterface

	virtual bool CanConsumeItem_Implementation(AActor* InteractorActor, FName ItemId, int32 Quantity) override;
	virtual void ConsumeItem_Implementation(AActor* InteractorActor, FName ItemId, int32 Quantity) override;

	//~End IInventoryItemConsumerInterface

	//~Begin IAimIndicatorPathProvider

	/** Builds path prediction input data for the current card throw setup. */
	virtual bool GetAimIndicatorPathData(FAimIndicatorPathData& OutPathData) const override;

	 //~End IAimIndicatorPathProvider

protected:

	/** Returns true if the character already owns a weapon of the given class */
	AShooterWeapon* FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const;

	/** Called when this character's HP is depleted */
	void Die();

	/** Called to allow Blueprint code to react to this character's death */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	/** Called after this character consumes an item from its inventory. */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory", meta = (DisplayName = "On Item Consumed"))
	void BP_OnItemConsumed(AActor* InteractorActor, FName ItemId, int32 Quantity);

	/** Called from the respawn timer to destroy this character and force the PC to respawn */
	void OnRespawn();

public:

	/** Returns true if the character is dead */
	bool IsDead() const;

private:
	bool bIsCardThrowingEnabled = false;
	bool bIsAimingCard = false;
	int32 NumCardsAvailable = 0;

	FAimIndicatorPathData CurrentThrowPathData;

	void UpdateThrowPath();
	void ThrowCard();
};
