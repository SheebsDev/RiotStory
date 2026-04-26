// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Input/GameplayInputTypeControllerInterface.h"
#include "Logging/LogMacros.h"
#include "RiotStoryCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UInteractorComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class ARiotStoryCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UInteractorComponent* Interactor;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

	/** Interact input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* InteractAction;

	/** Select1 input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* Select1Action;

	/** Select2 input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* Select2Action;

	/** Select3 input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* Select3Action;

	/** Select4 input action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* Select4Action;
	
	
public:
	ARiotStoryCharacter();

	/** Enables or disables a single gameplay input type on the owning controller. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void SetGameplayInputTypeEnabled(EGameplayInputType InputType, bool bEnabled);

	/** Enables or disables one or more gameplay input types on the owning controller. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void SetGameplayInputTypesEnabled(EGameplayInputType InputTypes, bool bEnabled);

	/** Enables or disables all gameplay input types on the owning controller. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void SetAllGameplayInputTypesEnabled(bool bEnabled);

	/** Returns true if the requested gameplay input type(s) are enabled on the owning controller. */
	UFUNCTION(BlueprintPure, Category="Input")
	virtual bool IsGameplayInputTypeEnabled(EGameplayInputType InputType) const;

protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Handles interact input */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoInteract();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSelect1();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSelect2();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSelect3();

	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSelect4();

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

private:
	void DoSelect(int SelectedIndex);

};

