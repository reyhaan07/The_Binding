#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BreathComponent.generated.h"

class USoundBase;

/** What Saskia's lungs are doing. */
UENUM(BlueprintType)
enum class EBreathState : uint8
{
	Normal      UMETA(DisplayName = "Normal"),       // quiet, even breathing
	Holding     UMETA(DisplayName = "Holding"),      // silent; air runs down
	Gasping     UMETA(DisplayName = "Gasping"),      // ran out: one loud gasp, can't hold for a moment
	Recovering  UMETA(DisplayName = "Recovering")    // out of breath, panting (loud) until air is back up
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBreathEventSignature);

/**
 * Saskia's breath (Story Bible 7.2 and 10, "Breath system").
 *
 * - Air runs from 1 (full lungs) to 0.
 * - Holding breath makes breathing silent but drains air: slowly when still (she is a freediver), faster when moving,
 *   much faster when running.
 * - At 0 she gasps: a loud noise spike, then panting while air recovers.
 * - Running and panting make breathing loud. Breath noise is reported to the AI hearing system with the tag "Breath",
 *   so the Bound One can hunt by it.
 * - ExtraDrainPerSecond is for the story systems still to come: the Bound One joined to her, Tess asleep and
 *   breathing through her (the third breath).
 *
 * Works on any ACharacter. The owner tells it when it is sprinting (bIsSprinting).
 */
UCLASS(ClassGroup = (Binding), meta = (BlueprintSpawnableComponent))
class THEBINDING_API UBreathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBreathComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Starts holding breath. Returns false if there is not enough air or she is mid-gasp. */
	UFUNCTION(BlueprintCallable, Category = "Breath")
	bool StartHold();

	/** Lets the breath out. */
	UFUNCTION(BlueprintCallable, Category = "Breath")
	void StopHold();

	/** 1 = full lungs, 0 = empty. */
	UFUNCTION(BlueprintPure, Category = "Breath")
	float GetAir() const { return Air; }

	UFUNCTION(BlueprintPure, Category = "Breath")
	EBreathState GetBreathState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Breath")
	bool IsHolding() const { return State == EBreathState::Holding; }

	/** Beats per minute. Drops a little at the start of a hold (the diving reflex), climbs as air runs out. */
	UFUNCTION(BlueprintPure, Category = "Breath")
	float GetHeartRate() const { return HeartRate; }

	/** How loud her breathing is right now, 0 (silent) to 1 (a gasp). */
	UFUNCTION(BlueprintPure, Category = "Breath")
	float GetBreathLoudness() const { return CurrentLoudness; }

	/** Set by the character every frame. */
	UPROPERTY(BlueprintReadWrite, Category = "Breath")
	bool bIsSprinting = false;

	/** Air taken from outside, per second (the Bound One joined to her, Tess asleep). 0 = none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath")
	float ExtraDrainPerSecond = 0.f;

	// ---- Tuning ----

	/** How long she can hold her breath standing still, from full lungs (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Tuning", meta = (ClampMin = "1"))
	float MaxHoldSeconds = 40.f;

	/** Holding drains this many times faster while walking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Tuning", meta = (ClampMin = "1"))
	float MovingHoldDrain = 1.8f;

	/** Holding drains this many times faster while running. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Tuning", meta = (ClampMin = "1"))
	float SprintingHoldDrain = 3.5f;

	/** Seconds to refill from empty to full while breathing normally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Tuning", meta = (ClampMin = "0.5"))
	float FullRecoverySeconds = 9.f;

	/** Running costs air (per second), so she ends up panting after a run. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Tuning", meta = (ClampMin = "0"))
	float SprintAirCostPerSecond = 0.05f;

	/** Below this much air she pants (Recovering) until it is back above it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Tuning", meta = (ClampMin = "0", ClampMax = "1"))
	float RecoveredAir = 0.6f;

	/** She needs at least this much air to start holding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Tuning", meta = (ClampMin = "0", ClampMax = "1"))
	float MinAirToHold = 0.2f;

	/** How long the gasp lasts before panting (seconds). She cannot hold her breath during it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Tuning", meta = (ClampMin = "0"))
	float GaspSeconds = 1.2f;

	// ---- Noise (what the Bound One hears; 0 = silent, 1 = a gasp) ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Noise")
	float CalmLoudness = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Noise")
	float WalkingLoudness = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Noise")
	float SprintingLoudness = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Noise")
	float PantingLoudness = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Noise")
	float GaspLoudness = 1.f;

	/** Crouching makes breathing this much quieter (multiplier). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Noise")
	float CrouchedLoudnessScale = 0.6f;

	/** How often breathing noise is reported to the AI (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Noise", meta = (ClampMin = "0.05"))
	float NoiseReportInterval = 0.4f;

	// ---- Heart ----

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Heart")
	float RestingHeartRate = 64.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Heart")
	float MaxHeartRate = 150.f;

	// ---- Sounds (optional: leave empty until you have them) ----

	/** The gasp when she runs out of air. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Sound")
	TObjectPtr<USoundBase> GaspSound;

	/** The slow breath out when she lets go of a hold. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Sound")
	TObjectPtr<USoundBase> ExhaleSound;

	/** One heartbeat. Plays on every beat while holding with little air left, or with a racing heart. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breath|Sound")
	TObjectPtr<USoundBase> HeartbeatSound;

	// ---- Events (bind these in Blueprint for sounds, camera effects, the HUD) ----

	UPROPERTY(BlueprintAssignable, Category = "Breath")
	FBreathEventSignature OnHoldStarted;

	UPROPERTY(BlueprintAssignable, Category = "Breath")
	FBreathEventSignature OnHoldReleased;

	UPROPERTY(BlueprintAssignable, Category = "Breath")
	FBreathEventSignature OnGasp;

	UPROPERTY(BlueprintAssignable, Category = "Breath")
	FBreathEventSignature OnRecovered;

	/** Air hit 0 while she was not holding (something is taking it). The story systems decide what happens. */
	UPROPERTY(BlueprintAssignable, Category = "Breath")
	FBreathEventSignature OnAirEmpty;

private:
	void Gasp();
	void UpdateLoudness(bool bMoving, bool bCrouched);
	void UpdateHeart(float DeltaTime);
	void ReportNoise(float Loudness, FName Tag);

	float Air = 1.f;
	EBreathState State = EBreathState::Normal;
	float GaspTimeLeft = 0.f;
	float CurrentLoudness = 0.f;
	float NoiseTimer = 0.f;
	float HeartRate = 64.f;
	float BeatPhase = 0.f;
	bool bAirEmptyFired = false;
};
