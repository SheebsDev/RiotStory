// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Game/RiotStoryGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include "Inventory/InventoryComponent.h"
#include "Variant_Shooter/Weapons/AimIndicatorComponent.h"
#include "Variant_Shooter/Weapons/BusinessCardProjectile.h"
#include "RiotStoryEventMessages.h"

AShooterCharacter::AShooterCharacter()
{
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));
	AimIndicator = CreateDefaultSubobject<UAimIndicatorComponent>(TEXT("Aim Indicator"));
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);

	//Set Card ThrowPath Defaults
	CurrentThrowPathData.MaxSimTime = 5.f;
	CurrentThrowPathData.SimFrequency = 0.f;
	CurrentThrowPathData.TraceChannel = ECC_Visibility;
	CurrentThrowPathData.bTraceWithCollision = true;
	CurrentThrowPathData.bTraceComplex = false;
	CurrentThrowPathData.bOverrideGravityZ = true;
	CurrentThrowPathData.GravityZ = -250.f;
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Simple Increasing the card throw speed
	//NOTE: May eventually want to move this to a component
	if (bIsAimingCard)
	{
		UpdateThrowPath();
	}
}

void AShooterCharacter::SetAvailableNumberOfCards(int32 Value)
{
	NumCardsAvailable = Value;
	OnAvailableCardCountChanged.Broadcast(Value);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;

	//Updating the HUD
	OnDamaged.Broadcast(1.0f);
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		//Aim/Zoom
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartZoom);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopZoom);

		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);
	}

}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// ignore if already dead
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// Reduce HP
	CurrentHP -= Damage;

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	// update the HUD
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));

	return Damage;
}

void AShooterCharacter::DoAim(float Yaw, float Pitch)
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoAim(Yaw, Pitch);
	}
}

void AShooterCharacter::DoStartZoom()
{
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StartZoom();
	}
}

void AShooterCharacter::DoStopZoom()
{
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StopZoom();
	}
}

void AShooterCharacter::DoMove(float Right, float Forward)
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoMove(Right, Forward);
	}
}

void AShooterCharacter::DoJumpStart()
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoJumpStart();
	}
}

void AShooterCharacter::DoJumpEnd()
{
	// only route inputs if the character is not dead
	if (!IsDead())
	{
		Super::DoJumpEnd();
	}
}

void AShooterCharacter::DoStartFiring()
{
	// fire the current weapon
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StartFiring();
		return;
	}

	if (bIsCardThrowingEnabled && CardProjectileClass)
	{
		if (NumCardsAvailable > 0)
		{
			bIsAimingCard = true;
			CardSpeed = 200.f; //reset speed
			AimIndicator->ShowAimIndicator();
		}
	}
}

void AShooterCharacter::DoStopFiring()
{
	// stop firing the current weapon
	if (CurrentWeapon && !IsDead())
	{
		CurrentWeapon->StopFiring();
		return;
	}

	if (bIsCardThrowingEnabled && CardProjectileClass)
	{
		bIsAimingCard = false;
		ThrowCard();
		AimIndicator->HideAimIndicator();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	// ensure we have at least two weapons two switch between
	if (OwnedWeapons.Num() > 1 && !IsDead())
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the last weapon?
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// loop back to the beginning of the array
			WeaponIndex = 0;
		}
		else {
			// select the next weapon index
			++WeaponIndex;
		}

		// set the new weapon as current
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// activate the new weapon
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	// stub
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// spawn the new weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// add the weapon to the owned list
			OwnedWeapons.Add(AddedWeapon);

			// if we have an existing weapon, deactivate it
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// switch to the new weapon
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// unused
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

bool AShooterCharacter::CanConsumeItem_Implementation(AActor* InteractorActor, FName ItemId, int32 Quantity)
{
	return IsValid(InteractorActor) && !ItemId.IsNone() && Quantity > 0;
}

void AShooterCharacter::ConsumeItem_Implementation(AActor* InteractorActor, FName ItemId, int32 Quantity)
{
	BP_OnItemConsumed(InteractorActor, ItemId, Quantity);
}

bool AShooterCharacter::GetAimIndicatorPathData(FAimIndicatorPathData& OutPathData) const
{
	OutPathData = CurrentThrowPathData;

	return true;
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;

}

void AShooterCharacter::Die()
{
	// deactivate the weapon
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	// increment the team score
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	// grant the death tag to the character
	Tags.Add(DeathTag);
		
	// stop character movement
	GetCharacterMovement()->StopMovementImmediately();

	// disable gameplay input groups on the owning controller
	SetAllGameplayInputTypesEnabled(false);

	// disable controls
	DisableInput(nullptr);

	// reset the bullet counter UI
	OnBulletCountUpdated.Broadcast(0, 0);

	// call the BP handler
	BP_OnDeath();

	// schedule character respawn
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}

void AShooterCharacter::OnRespawn()
{
	// destroy the character to force the PC to respawn
	Destroy();
}

bool AShooterCharacter::IsDead() const
{
	// the character is dead if their current HP drops to zero
	return CurrentHP <= 0.0f;
}

void AShooterCharacter::UpdateThrowPath()
{
	CardSpeed += SpeedGain;
	CardSpeed = FMath::Clamp(CardSpeed, 0.f, MaxCardSpeed);

	const FVector& Forward = GetActorForwardVector();
	const FQuat& PitchQuat = FQuat(GetActorRightVector(), FMath::DegreesToRadians(-30.f));
	FVector AngledForward = PitchQuat.RotateVector(Forward);

	FVector CurrentVelocity = AngledForward * CardSpeed;
	FVector RightSeparation = GetActorRightVector() * 10.f;
	CurrentThrowPathData.StartLocation = GetFirstPersonCameraComponent()->GetComponentLocation() - RightSeparation;
	CurrentThrowPathData.StartLocation.Z -= 20.f;
	CurrentThrowPathData.LaunchVelocity = CurrentVelocity;
}

void AShooterCharacter::ThrowCard()
{
	if (NumCardsAvailable <= 0) return;

	SetAvailableNumberOfCards(NumCardsAvailable-1);

	// Ensure throw data is current even if input started/stopped in a single frame.
	UpdateThrowPath();

	const FVector LaunchDirection = CurrentThrowPathData.LaunchVelocity.GetSafeNormal();
	if (LaunchDirection.IsNearlyZero())
	{
		return;
	}

	const FTransform ProjectileTransform(LaunchDirection.Rotation(), CurrentThrowPathData.StartLocation, FVector::OneVector);

	// spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	UWorld* World = GetWorld();
	if (ABusinessCardProjectile* Projectile = World->SpawnActor<ABusinessCardProjectile>(CardProjectileClass, ProjectileTransform, SpawnParams))
	{
		Projectile->SetLaunchVelocity(CurrentThrowPathData.LaunchVelocity);
	}

	UGameplayMessageSubsystem::Get(this).BroadcastMessage(
		RiotStoryGameplayTags::TAG_GameEvent_CardThrowGame_CardThrown,
		FGameEventCardThrownMessage()
	);
}
