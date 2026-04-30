// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterPlayerController.h"
#include "Conversation/ConversationRuntimeComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "UI/ShooterBulletCounterUI.h"
#include "UI/ModeTransitionUI.h"
#include "UI/CardGameUI.h"
#include "Game/RiotStoryGameplayTags.h"
#include "RiotStory.h"

AShooterPlayerController::AShooterPlayerController()
{
	// Preserve shooter behavior from before inheriting ARiotStoryPlayerController.
	PlayerCameraManagerClass = APlayerCameraManager::StaticClass();
}

AShooterCharacter* AShooterPlayerController::GetShooterPawn()
{
	return Cast<AShooterCharacter>(GetPawn());
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ModeTransitionCommandListener = UGameplayMessageSubsystem::Get(this).RegisterListener(
		RiotStoryGameplayTags::TAG_GameCommand_StartModeTransition,
		this,
		&AShooterPlayerController::HandleStartTransitionCommand
	);
}

void AShooterPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//Cleanup Listeners
	UGameplayMessageSubsystem::Get(this).UnregisterListener(ModeTransitionCommandListener);
}

void AShooterPlayerController::LoadPlayerHUD()
{
	Super::LoadPlayerHUD();

	// create the bullet counter widget and add it to the screen
	BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

	if (BulletCounterUI)
	{
		//BulletCounterUI->AddToPlayerScreen(0);
	}
	else
	{
		UE_LOG(LogRiotStory, Error, TEXT("Could not spawn bullet counter widget."));
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// is this a shooter character?
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// add the player tag
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// subscribe to the pawn's delegates
		ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);
		ShooterCharacter->OnAvailableCardCountChanged.AddDynamic(this, &AShooterPlayerController::OnCardCountUpdated);

		// force update the life bar
		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}

	SetAllGameplayInputTypesEnabled(true);
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	if (UConversationRuntimeComponent* const ConversationRuntimeTemp = GetConversationRuntime();
		IsValid(ConversationRuntimeTemp) && ConversationRuntimeTemp->IsConversationActive())
	{
		ConversationRuntimeTemp->EndConversation(EConversationEndReason::Cancelled);
	}

	SetAllGameplayInputTypesEnabled(true);

	// reset the bullet counter HUD
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// select a random player start
		AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

		// spawn a character at the player start
		const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

		if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
		{
			// possess the character
			Possess(RespawnedCharacter);
		}
	}
}

void AShooterPlayerController::OnCardCountUpdated(int32 NewCardCount)
{
	if (CardGameUI)
	{
		CardGameUI->BP_UpdateCardCount(NewCardCount);
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

UModeTransitionUI* AShooterPlayerController::GetOrCreateModeTransitionUI()
{
	if (!ModeTransitionUI && ModeTransitionWidgetClass)
	{
		ModeTransitionUI = CreateWidget<UModeTransitionUI>(this, ModeTransitionWidgetClass);
	}

	return ModeTransitionUI;
}

UCardGameUI* AShooterPlayerController::GetOrCreateCardGameUI()
{
	if (!CardGameUI && CardGameUIClass)
	{
		CardGameUI = CreateWidget<UCardGameUI>(this, CardGameUIClass);
	}

	return CardGameUI;
}

void AShooterPlayerController::HandleStartTransitionCommand(FGameplayTag Changed, const FGameCommandModeTransitionMessage& Message)
{
	UModeTransitionUI* const TransitionUI = GetOrCreateModeTransitionUI();
	if (!IsValid(TransitionUI))
	{
		UE_LOG(LogRiotStory, Error, TEXT("Could not spawn mode transition widget."));
		return;
	}

	SetAllGameplayInputTypesEnabled(false);
	TransitionUI->SetTransitionModes(Message.NewMode, Message.CurrentMode);
	TransitionUI->AddToViewport(1); // Keep transition on top while gameplay widgets change.
	TransitionUI->TriggerStartTransition();
}
