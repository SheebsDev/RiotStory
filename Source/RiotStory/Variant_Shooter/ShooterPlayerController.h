// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/RiotStoryPlayerController.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "RiotStoryCommandMessages.h"
#include "ShooterPlayerController.generated.h"

class AShooterCharacter;
class UShooterBulletCounterUI;
class UCardGameUI;
class UModeTransitionUI;

/**
 *  Simple PlayerController for a first person shooter game
 *  Respawns the player pawn when it's destroyed
 */
UCLASS(abstract, config="Game")
class RIOTSTORY_API AShooterPlayerController : public ARiotStoryPlayerController
{
	GENERATED_BODY()

public:

	AShooterPlayerController();

	UFUNCTION(BlueprintPure, Category="Shooter")
	AShooterCharacter* GetShooterPawn();

	UModeTransitionUI* GetOrCreateModeTransitionUI();
	UCardGameUI* GetOrCreateCardGameUI();
	
protected:

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	/** Type of bullet counter UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UCardGameUI> CardGameUIClass;

	/** Tag to grant the possessed pawn to flag it as the player */
	UPROPERTY(EditAnywhere, Category="Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	/** Pointer to the bullet counter UI widget */
	UPROPERTY()
	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;

	/** Reference to the Card Game UI widget */
	UPROPERTY()
	TObjectPtr<UCardGameUI> CardGameUI;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void LoadPlayerHUD() override;

	virtual void OnPossess(APawn* InPawn) override;

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	/** Called when the bullet count on the possessed pawn is updated */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** Called when the card count on the pawn is updated */
	UFUNCTION()
	void OnCardCountUpdated(int NewCardCount);

	/** Called when the possessed pawn is damaged */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

private:
	FGameplayMessageListenerHandle ModeTransitionCommandListener;

	void HandleStartTransitionCommand(FGameplayTag Changed, const FGameCommandModeTransitionMessage& Message);
};
