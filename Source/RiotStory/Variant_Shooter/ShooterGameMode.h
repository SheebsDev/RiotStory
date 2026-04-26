// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class ACardBucketDirector;
class UShooterUI;

/**
 *  Simple GameMode for a first person shooter game
 *  Manages game UI
 *  Keeps track of team scores
 */
UCLASS(abstract)
class RIOTSTORY_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter")
	TSubclassOf<UShooterUI> ShooterUIClass;

	/** Pointer to the UI widget */
	TObjectPtr<UShooterUI> ShooterUI;

	/** Map of scores by team ID */
	TMap<uint8, int32> TeamScores;

	/** Runtime class used to spawn the card bucket director actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Card Buckets")
	TSubclassOf<ACardBucketDirector> CardBucketDirectorClass;

	/** Runtime instance used to execute bucket spawn/despawn operations. */
	UPROPERTY(Transient)
	TObjectPtr<ACardBucketDirector> CardBucketDirector;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);

	/** Creates the bucket director when missing and authority allows. */
	UFUNCTION(BlueprintCallable, Category="Card Buckets")
	ACardBucketDirector* CreateCardBucketDirector();

	/** Returns the current bucket director instance without spawning one. */
	UFUNCTION(BlueprintPure, Category="Card Buckets")
	ACardBucketDirector* GetCardBucketDirector() const { return CardBucketDirector; }

	/** Destroys the current bucket director instance if present. */
	UFUNCTION(BlueprintCallable, Category="Card Buckets")
	void DestroyCardBucketDirector();
};
