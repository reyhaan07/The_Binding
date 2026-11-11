#include "Player/BreathComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "Sound/SoundBase.h"

UBreathComponent::UBreathComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

bool UBreathComponent::StartHold()
{
	if (State == EBreathState::Holding)
	{
		return true;
	}
	if (State == EBreathState::Gasping || Air < MinAirToHold)
	{
		return false;
	}
	State = EBreathState::Holding;
	OnHoldStarted.Broadcast();
	return true;
}

void UBreathComponent::StopHold()
{
	if (State != EBreathState::Holding)
	{
		return;
	}
	State = (Air < RecoveredAir) ? EBreathState::Recovering : EBreathState::Normal;
	if (ExhaleSound)
	{
		UGameplayStatics::PlaySound2D(this, ExhaleSound);
	}
	OnHoldReleased.Broadcast();
}

void UBreathComponent::Gasp()
{
	State = EBreathState::Gasping;
	GaspTimeLeft = GaspSeconds;
	ReportNoise(GaspLoudness, TEXT("Gasp"));
	if (GaspSound)
	{
		UGameplayStatics::PlaySound2D(this, GaspSound);
	}
	OnGasp.Broadcast();
}

void UBreathComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	const float Speed = Character ? Character->GetVelocity().Size2D() : 0.f;
	const bool bMoving = Speed > 10.f;
	const bool bCrouched = Character && Character->bIsCrouched;
	const bool bRunning = bIsSprinting && bMoving;
	const float Refill = DeltaTime / FullRecoverySeconds;

	switch (State)
	{
	case EBreathState::Holding:
	{
		const float Drain = bRunning ? SprintingHoldDrain : (bMoving ? MovingHoldDrain : 1.f);
		Air -= DeltaTime * Drain / MaxHoldSeconds;
		if (Air <= 0.f)
		{
			Air = 0.f;
			Gasp();
		}
		break;
	}
	case EBreathState::Gasping:
		Air += Refill;
		GaspTimeLeft -= DeltaTime;
		if (GaspTimeLeft <= 0.f)
		{
			State = EBreathState::Recovering;
		}
		break;

	case EBreathState::Normal:
	case EBreathState::Recovering:
		// Running uses air faster than she can take it in: after a run she is out of breath.
		Air += bRunning ? -SprintAirCostPerSecond * DeltaTime : Refill;
		if (State == EBreathState::Normal && Air < RecoveredAir && bRunning)
		{
			State = EBreathState::Recovering;
		}
		else if (State == EBreathState::Recovering && Air >= RecoveredAir)
		{
			State = EBreathState::Normal;
			OnRecovered.Broadcast();
		}
		break;
	}

	// Something else taking her breath (the Bound One, Tess asleep).
	if (ExtraDrainPerSecond > 0.f)
	{
		Air -= ExtraDrainPerSecond * DeltaTime;
	}
	Air = FMath::Clamp(Air, 0.f, 1.f);

	if (Air <= 0.f && State != EBreathState::Holding && State != EBreathState::Gasping)
	{
		if (!bAirEmptyFired)
		{
			bAirEmptyFired = true;
			OnAirEmpty.Broadcast();
		}
	}
	else if (Air > 0.05f)
	{
		bAirEmptyFired = false;
	}

	UpdateLoudness(bMoving, bCrouched);

	NoiseTimer -= DeltaTime;
	if (NoiseTimer <= 0.f)
	{
		NoiseTimer = NoiseReportInterval;
		if (CurrentLoudness > 0.01f && State != EBreathState::Gasping)   // the gasp reported itself
		{
			ReportNoise(CurrentLoudness, TEXT("Breath"));
		}
	}

	UpdateHeart(DeltaTime);
}

void UBreathComponent::UpdateLoudness(bool bMoving, bool bCrouched)
{
	float Loudness = 0.f;
	switch (State)
	{
	case EBreathState::Holding:
		Loudness = 0.f;
		break;
	case EBreathState::Gasping:
		Loudness = GaspLoudness;
		break;
	case EBreathState::Recovering:
	{
		// The emptier her lungs, the harder she pants.
		const float OutOfBreath = 1.f - FMath::Clamp(Air / FMath::Max(RecoveredAir, 0.01f), 0.f, 1.f);
		Loudness = FMath::Lerp(PantingLoudness * 0.6f, PantingLoudness, OutOfBreath);
		break;
	}
	case EBreathState::Normal:
		Loudness = bIsSprinting && bMoving ? SprintingLoudness : (bMoving ? WalkingLoudness : CalmLoudness);
		break;
	}
	if (bCrouched && State != EBreathState::Gasping)
	{
		Loudness *= CrouchedLoudnessScale;
	}
	CurrentLoudness = Loudness;
}

void UBreathComponent::UpdateHeart(float DeltaTime)
{
	float Stress = (1.f - Air) * 0.8f;
	if (bIsSprinting)
	{
		Stress += 0.5f;
	}
	if (State == EBreathState::Gasping)
	{
		Stress += 0.6f;
	}
	float Target = FMath::Lerp(RestingHeartRate, MaxHeartRate, FMath::Clamp(Stress, 0.f, 1.f));
	if (State == EBreathState::Holding && Air > 0.4f)
	{
		// The diving reflex: a trained freediver's heart slows when she holds her breath.
		Target = RestingHeartRate - 8.f;
	}
	HeartRate = FMath::FInterpTo(HeartRate, Target, DeltaTime, 0.8f);

	BeatPhase += DeltaTime * HeartRate / 60.f;
	if (BeatPhase >= 1.f)
	{
		BeatPhase -= 1.f;
		const bool bAudible = (State == EBreathState::Holding && Air < 0.35f) || HeartRate > 110.f;
		if (bAudible && HeartbeatSound)
		{
			const float Volume = static_cast<float>(FMath::GetMappedRangeValueClamped(FVector2D(0.35, 0.0), FVector2D(0.4, 1.0), static_cast<double>(Air)));
			UGameplayStatics::PlaySound2D(this, HeartbeatSound, Volume);
		}
	}
}

void UBreathComponent::ReportNoise(float Loudness, FName Tag)
{
	if (AActor* Owner = GetOwner())
	{
		UAISense_Hearing::ReportNoiseEvent(this, Owner->GetActorLocation(), Loudness, Owner, 0.f, Tag);
	}
}
