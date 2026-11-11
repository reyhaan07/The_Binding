#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SaskiaCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USpotLightComponent;
class UInputAction;
class UInputMappingContext;
class UBreathComponent;
class UInteractableComponent;
class USoundBase;

/**
 * Saskia: the player. First person, no visible body yet (arms come later with real motion-captured animation).
 *
 * Controls (built in code, so the project needs no input assets; assign your own in a Blueprint child to override):
 *   WASD / left stick ............ move
 *   Mouse / right stick .......... look
 *   Left Shift / L3 .............. run (hold)
 *   C or Left Ctrl / B ........... crouch (toggle)
 *   Space / left trigger ......... hold breath (hold)
 *   F / Y ........................ flashlight
 *   E / A ........................ use (doors, lamps, the phone)
 *
 * Every footstep is reported to the AI hearing system with the tag "Footstep"; her breathing is reported by the
 * Breath component with the tag "Breath" (and "Gasp").
 */
UCLASS()
class THEBINDING_API ASaskiaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASaskiaCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// ---- Flashlight ----

	UFUNCTION(BlueprintCallable, Category = "Saskia|Flashlight")
	void SetFlashlightOn(bool bOn);

	UFUNCTION(BlueprintPure, Category = "Saskia|Flashlight")
	bool IsFlashlightOn() const { return bFlashlightOn; }

	/** Makes the torch stutter for a while: for scares, or when the Bound One is close. */
	UFUNCTION(BlueprintCallable, Category = "Saskia|Flashlight")
	void FlickerFlashlight(float Seconds);

	// ---- Queries ----

	UFUNCTION(BlueprintPure, Category = "Saskia")
	UBreathComponent* GetBreath() const { return Breath; }

	/** The usable thing she is looking at, or null. */
	UFUNCTION(BlueprintPure, Category = "Saskia")
	UInteractableComponent* GetFocusedInteractable() const { return FocusedInteractable.Get(); }

	UFUNCTION(BlueprintPure, Category = "Saskia")
	bool IsSprinting() const { return bSprinting; }

protected:
	// ---- Components ----

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saskia")
	TObjectPtr<UCameraComponent> Camera;

	/** Holds the torch a little to the right and below her eyes, and makes it trail her view slightly, like a hand. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saskia")
	TObjectPtr<USpringArmComponent> FlashlightArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saskia")
	TObjectPtr<USpotLightComponent> Flashlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saskia")
	TObjectPtr<UBreathComponent> Breath;

	// ---- Movement tuning (cm/s) ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Movement")
	float WalkSpeed = 240.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Movement")
	float SprintSpeed = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Movement")
	float CrouchSpeed = 120.f;

	/** Eye height above the middle of her body (cm). 64 puts her eyes about 1.52 m above the floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Camera")
	float EyeHeight = 64.f;

	/** Mouse look speed multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Camera")
	float MouseSensitivity = 1.f;

	/** Gamepad look speed (degrees per second at full stick). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Camera")
	float StickLookRate = 110.f;

	/** How much her view rises and falls with each step (cm). Walking / running / crouched. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Camera")
	FVector HeadBob = FVector(1.1f, 2.0f, 0.6f);

	// ---- Footsteps ----

	/** Stride length (cm). Walking / running / crouched. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Footsteps")
	FVector StrideLength = FVector(72.f, 95.f, 50.f);

	/** How loud a footstep is to the Bound One (0-1). Walking / running / crouched. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Footsteps")
	FVector FootstepLoudness = FVector(0.3f, 0.85f, 0.08f);

	/** Optional footstep sound (use a sound with a few random variations). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Footsteps")
	TObjectPtr<USoundBase> FootstepSound;

	// ---- Flashlight tuning ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Flashlight")
	float FlashlightLumens = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Flashlight")
	TObjectPtr<USoundBase> FlashlightClickSound;

	// ---- Interaction ----

	/** How far she can reach to use things (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saskia|Interaction")
	float InteractReach = 190.f;

	// ---- Input (leave empty to use the built-in controls listed above) ----

	UPROPERTY(EditDefaultsOnly, Category = "Saskia|Input")
	TObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Saskia|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Saskia|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Saskia|Input")
	TObjectPtr<UInputAction> StickLookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Saskia|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Saskia|Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Saskia|Input")
	TObjectPtr<UInputAction> HoldBreathAction;

	UPROPERTY(EditDefaultsOnly, Category = "Saskia|Input")
	TObjectPtr<UInputAction> FlashlightAction;

	UPROPERTY(EditDefaultsOnly, Category = "Saskia|Input")
	TObjectPtr<UInputAction> InteractAction;

private:
	/** Creates the default controls if no input assets were assigned. */
	void EnsureInput();

	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnStickLook(const FInputActionValue& Value);
	void OnSprintPressed();
	void OnSprintReleased();
	void OnCrouchPressed();
	void OnHoldBreathPressed();
	void OnHoldBreathReleased();
	void OnFlashlightPressed();
	void OnInteractPressed();

	void UpdateMovement();
	void UpdateFootstepsAndHeadBob(float DeltaSeconds);
	void UpdateCamera(float DeltaSeconds);
	void UpdateFlashlight(float DeltaSeconds);
	void UpdateInteractionFocus();

	TWeakObjectPtr<UInteractableComponent> FocusedInteractable;

	bool bWantsToSprint = false;
	bool bSprinting = false;
	bool bFlashlightOn = true;

	float StepDistance = 0.f;
	int32 StepCount = 0;
	FVector2D BobOffset = FVector2D::ZeroVector;        // X = sideways, Y = up/down (cm)
	float CrouchEyeOffset = 0.f;
	float BreathPhase = 0.f;
	float BreathOffset = 0.f;

	float FlickerTimeLeft = 0.f;
	float FlickerStepTimer = 0.f;
};
