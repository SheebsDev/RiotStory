// Copyright Epic Games, Inc. All Rights Reserved.

#include "RiotStoryCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Conversation/ConversationRuntimeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/InteractorComponent.h"
#include "GameFramework/Controller.h"
#include "RiotStory.h"

ARiotStoryCharacter::ARiotStoryCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->SetVisibility(false);

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 98.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	Interactor = CreateDefaultSubobject<UInteractorComponent>(TEXT("Interactor"));

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void ARiotStoryCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARiotStoryCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ARiotStoryCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARiotStoryCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARiotStoryCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ARiotStoryCharacter::LookInput);

		// Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ARiotStoryCharacter::DoInteract);

		//Conversation Selection
		EnhancedInputComponent->BindAction(Select1Action, ETriggerEvent::Triggered, this, &ARiotStoryCharacter::DoSelect1);
		EnhancedInputComponent->BindAction(Select2Action, ETriggerEvent::Triggered, this, &ARiotStoryCharacter::DoSelect2);
		EnhancedInputComponent->BindAction(Select3Action, ETriggerEvent::Triggered, this, &ARiotStoryCharacter::DoSelect3);
		EnhancedInputComponent->BindAction(Select4Action, ETriggerEvent::Triggered, this, &ARiotStoryCharacter::DoSelect4);
	}
	else
	{
		UE_LOG(LogRiotStory, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ARiotStoryCharacter::SetGameplayInputTypeEnabled(const EGameplayInputType InputType, const bool bEnabled)
{
	if (IGameplayInputTypeControllerInterface* const InputController = Cast<IGameplayInputTypeControllerInterface>(GetController()))
	{
		InputController->SetGameplayInputTypeEnabled(InputType, bEnabled);
	}
}

void ARiotStoryCharacter::SetGameplayInputTypesEnabled(const EGameplayInputType InputTypes, const bool bEnabled)
{
	if (IGameplayInputTypeControllerInterface* const InputController = Cast<IGameplayInputTypeControllerInterface>(GetController()))
	{
		InputController->SetGameplayInputTypesEnabled(InputTypes, bEnabled);
	}
}

void ARiotStoryCharacter::SetAllGameplayInputTypesEnabled(const bool bEnabled)
{
	if (IGameplayInputTypeControllerInterface* const InputController = Cast<IGameplayInputTypeControllerInterface>(GetController()))
	{
		InputController->SetAllGameplayInputTypesEnabled(bEnabled);
	}
}

bool ARiotStoryCharacter::IsGameplayInputTypeEnabled(const EGameplayInputType InputType) const
{
	if (const IGameplayInputTypeControllerInterface* const InputController = Cast<IGameplayInputTypeControllerInterface>(GetController()))
	{
		return InputController->IsGameplayInputTypeEnabled(InputType);
	}

	return false;
}


void ARiotStoryCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ARiotStoryCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ARiotStoryCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ARiotStoryCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ARiotStoryCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ARiotStoryCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void ARiotStoryCharacter::DoInteract()
{
	if (AController* const OwnerController = GetController())
	{
		if (UConversationRuntimeComponent* const ConversationRuntime = OwnerController->FindComponentByClass<UConversationRuntimeComponent>())
		{
			if (ConversationRuntime->IsConversationActive())
			{
				ConversationRuntime->AdvanceConversation();
				return;
			}
		}
	}

	if (Interactor)
	{
		Interactor->TryInteract();
	}
}

void ARiotStoryCharacter::DoSelect1()
{
	DoSelect(0);
}

void ARiotStoryCharacter::DoSelect2()
{
	DoSelect(1);
}

void ARiotStoryCharacter::DoSelect3()
{
	DoSelect(2);
}

void ARiotStoryCharacter::DoSelect4()
{
	DoSelect(3);
}

void ARiotStoryCharacter::DoSelect(int SelectedIndex)
{
	if (AController* const OwnerController = GetController())
	{
		if (UConversationRuntimeComponent* const ConversationRuntime = OwnerController->FindComponentByClass<UConversationRuntimeComponent>())
		{
			if (ConversationRuntime->IsConversationActive())
			{
				ConversationRuntime->SelectConversationResponse(SelectedIndex);
				return;
			}
		}
	}
}
