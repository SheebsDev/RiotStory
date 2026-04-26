// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "ShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "RiotStory.h"
#include "Variant_Shooter/CardBucketDirector.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	// create the UI
	/*ShooterUI = CreateWidget<UShooterUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), ShooterUIClass);
	ShooterUI->AddToViewport(0);*/
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	// retrieve the team score if any
	int32 Score = 0;
	if (int32* FoundScore = TeamScores.Find(TeamByte))
	{
		Score = *FoundScore;
	}

	// increment the score for the given team
	++Score;
	TeamScores.Add(TeamByte, Score);

	// update the UI
	ShooterUI->BP_UpdateScore(TeamByte, Score);
}

ACardBucketDirector* AShooterGameMode::CreateCardBucketDirector()
{
	if (IsValid(CardBucketDirector))
	{
		return CardBucketDirector;
	}

	checkf(CardBucketDirectorClass, TEXT("CardBucketDirector class was never set!"));

	UWorld* const World = GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CardBucketDirector = World->SpawnActor<ACardBucketDirector>(CardBucketDirectorClass, FTransform::Identity, SpawnParams);
	if (!IsValid(CardBucketDirector))
	{
		UE_LOG(LogRiotStory, Warning, TEXT("ShooterGameMode failed to spawn CardBucketDirector from class '%s'."), *CardBucketDirectorClass->GetName());
	}

	return CardBucketDirector;
}

void AShooterGameMode::DestroyCardBucketDirector()
{
	if (!CardBucketDirector)
	{
		return;
	}

	if (IsValid(CardBucketDirector))
	{
		CardBucketDirector->Destroy();
	}

	CardBucketDirector = nullptr;
}
