#include "Player/SaskiaCharacter.h"

#include "TheBinding.h"
#include "Player/BreathComponent.h"
#include "Interaction/InteractableComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "Sound/SoundBase.h"

namespace
{
	/** Adds one key, optionally turning a 1D key into the forward axis (swizzle) and/or flipping it (negate). */
	void AddKey(UInputMappingContext* Context, UInputAction* Action, const FKey& Key, bool bSwizzleYX = false,
	            bool bNegate = false, float DeadZone = 0.f)
	{
		// Finish with the returned mapping before the next MapKey call: the array may move in memory.
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
		if (bSwizzleYX)
		{
			UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(Context);
			Swizzle->Order = EInputAxisSwizzle::YXZ;
			Mapping.Modifiers.Add(Swizzle);
		}
		if (bNegate)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(Context));
		}
		if (DeadZone > 0.f)
		{
			UInputModifierDeadZone* Zone = NewObject<UInputModifierDeadZone>(Context);
			Zone->LowerThreshold = DeadZone;
			Mapping.Modifiers.Add(Zone);
		}
	}
}

ASaskiaCharacter::ASaskiaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, EyeHeight));
	Camera->bUsePawnControlRotation = true;
	Camera->SetFieldOfView(80.f);

	FlashlightArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FlashlightArm"));
	FlashlightArm->SetupAttachment(Camera);
	FlashlightArm->SetRelativeLocation(FVector(12.f, 16.f, -14.f));
	FlashlightArm->TargetArmLength = 0.f;
	FlashlightArm->bDoCollisionTest = false;
	FlashlightArm->bUsePawnControlRotation = false;
	FlashlightArm->bEnableCameraRotationLag = true;   // the beam trails her view a touch, like a torch in a hand
	FlashlightArm->CameraRotationLagSpeed = 11.f;

	Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(FlashlightArm, USpringArmComponent::SocketName);
	Flashlight->IntensityUnits = ELightUnits::Lumens;
	Flashlight->Intensity = FlashlightLumens;
	Flashlight->InnerConeAngle = 12.f;
	Flashlight->OuterConeAngle = 26.f;
	Flashlight->AttenuationRadius = 2500.f;
	Flashlight->SourceRadius = 2.f;
	Flashlight->bUseTemperature = true;
	Flashlight->Temperature = 4300.f;                 // an old torch: a little warm
	Flashlight->SetCastShadows(true);

	Breath = CreateDefaultSubobject<UBreathComponent>(TEXT("Breath"));

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->MaxWalkSpeed = WalkSpeed;
	Move->MaxWalkSpeedCrouched = CrouchSpeed;
	Move->MaxAcceleration = 1100.f;                    // a body with weight: no instant starts
	Move->BrakingDecelerationWalking = 1300.f;
	Move->GroundFriction = 8.f;
	Move->GetNavAgentPropertiesRef().bCanCrouch = true;
	Move->SetCrouchedHalfHeight(50.f);
	Move->bCanWalkOffLedgesWhenCrouching = true;
	Move->bOrientRotationToMovement = false;
}

void ASaskiaCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Apply the tuning values (a Blueprint child may have changed them).
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	Flashlight->SetIntensity(FlashlightLumens);
	SetFlashlightOn(bFlashlightOn);
}

// ---------------------------------------------------------------------------------------------------------------- input

void ASaskiaCharacter::EnsureInput()
{
	if (InputMapping)
	{
		return;   // a Blueprint child assigned its own input assets
	}

	InputMapping = NewObject<UInputMappingContext>(this, TEXT("IMC_Saskia_Default"));

	auto MakeAction = [this](UInputAction*& Out, const TCHAR* Name, EInputActionValueType Type)
	{
		if (!Out)
		{
			Out = NewObject<UInputAction>(this, Name);
			Out->ValueType = Type;
		}
	};
	UInputAction* Move = MoveAction;
	UInputAction* Look = LookAction;
	UInputAction* StickLook = StickLookAction;
	UInputAction* Sprint = SprintAction;
	UInputAction* CrouchA = CrouchAction;
	UInputAction* Hold = HoldBreathAction;
	UInputAction* Torch = FlashlightAction;
	UInputAction* Use = InteractAction;
	MakeAction(Move, TEXT("IA_Move"), EInputActionValueType::Axis2D);
	MakeAction(Look, TEXT("IA_Look"), EInputActionValueType::Axis2D);
	MakeAction(StickLook, TEXT("IA_StickLook"), EInputActionValueType::Axis2D);
	MakeAction(Sprint, TEXT("IA_Sprint"), EInputActionValueType::Boolean);
	MakeAction(CrouchA, TEXT("IA_Crouch"), EInputActionValueType::Boolean);
	MakeAction(Hold, TEXT("IA_HoldBreath"), EInputActionValueType::Boolean);
	MakeAction(Torch, TEXT("IA_Flashlight"), EInputActionValueType::Boolean);
	MakeAction(Use, TEXT("IA_Interact"), EInputActionValueType::Boolean);
	MoveAction = Move;
	LookAction = Look;
	StickLookAction = StickLook;
	SprintAction = Sprint;
	CrouchAction = CrouchA;
	HoldBreathAction = Hold;
	FlashlightAction = Torch;
	InteractAction = Use;

	// Move: W/S are the forward axis (Y), A/D the side axis (X).
	AddKey(InputMapping, Move, EKeys::W, true);
	AddKey(InputMapping, Move, EKeys::S, true, true);
	AddKey(InputMapping, Move, EKeys::A, false, true);
	AddKey(InputMapping, Move, EKeys::D);
	AddKey(InputMapping, Move, EKeys::Gamepad_Left2D, false, false, 0.2f);

	AddKey(InputMapping, Look, EKeys::Mouse2D);
	AddKey(InputMapping, StickLook, EKeys::Gamepad_Right2D, false, false, 0.2f);

	AddKey(InputMapping, Sprint, EKeys::LeftShift);
	AddKey(InputMapping, Sprint, EKeys::Gamepad_LeftThumbstick);
	AddKey(InputMapping, CrouchA, EKeys::C);
	AddKey(InputMapping, CrouchA, EKeys::LeftControl);
	AddKey(InputMapping, CrouchA, EKeys::Gamepad_FaceButton_Right);
	AddKey(InputMapping, Hold, EKeys::SpaceBar);
	AddKey(InputMapping, Hold, EKeys::Gamepad_LeftTrigger);
	AddKey(InputMapping, Torch, EKeys::F);
	AddKey(InputMapping, Torch, EKeys::Gamepad_FaceButton_Top);
	AddKey(InputMapping, Use, EKeys::E);
	AddKey(InputMapping, Use, EKeys::Gamepad_FaceButton_Bottom);
}

void ASaskiaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	EnsureInput();

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		UE_LOG(LogBinding, Error, TEXT("Saskia needs Enhanced Input (Project Settings > Input > Default Input Component Class)."));
		return;
	}
	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASaskiaCharacter::OnMove);
	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASaskiaCharacter::OnLook);
	Input->BindAction(StickLookAction, ETriggerEvent::Triggered, this, &ASaskiaCharacter::OnStickLook);
	Input->BindAction(SprintAction, ETriggerEvent::Started, this, &ASaskiaCharacter::OnSprintPressed);
	Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASaskiaCharacter::OnSprintReleased);
	Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASaskiaCharacter::OnCrouchPressed);
	Input->BindAction(HoldBreathAction, ETriggerEvent::Started, this, &ASaskiaCharacter::OnHoldBreathPressed);
	Input->BindAction(HoldBreathAction, ETriggerEvent::Completed, this, &ASaskiaCharacter::OnHoldBreathReleased);
	Input->BindAction(FlashlightAction, ETriggerEvent::Started, this, &ASaskiaCharacter::OnFlashlightPressed);
	Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ASaskiaCharacter::OnInteractPressed);
}

void ASaskiaCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();   // sets up the input component (SetupPlayerInputComponent) first

	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
	{
		EnsureInput();
		Subsystem->RemoveMappingContext(InputMapping);
		Subsystem->AddMappingContext(InputMapping, 0);
	}
}

void ASaskiaCharacter::OnMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void ASaskiaCharacter::OnLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X * MouseSensitivity);
	AddControllerPitchInput(-Axis.Y * MouseSensitivity);
}

void ASaskiaCharacter::OnStickLook(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	const float Step = StickLookRate * GetWorld()->GetDeltaSeconds();
	AddControllerYawInput(Axis.X * Step);
	AddControllerPitchInput(-Axis.Y * Step);
}

void ASaskiaCharacter::OnSprintPressed()
{
	bWantsToSprint = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void ASaskiaCharacter::OnSprintReleased()
{
	bWantsToSprint = false;
}

void ASaskiaCharacter::OnCrouchPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		bWantsToSprint = false;
		Crouch();
	}
}

void ASaskiaCharacter::OnHoldBreathPressed()
{
	Breath->StartHold();
}

void ASaskiaCharacter::OnHoldBreathReleased()
{
	Breath->StopHold();
}

void ASaskiaCharacter::OnFlashlightPressed()
{
	SetFlashlightOn(!bFlashlightOn);
	if (FlashlightClickSound)
	{
		UGameplayStatics::PlaySound2D(this, FlashlightClickSound);
	}
}

void ASaskiaCharacter::OnInteractPressed()
{
	if (UInteractableComponent* Target = FocusedInteractable.Get())
	{
		Target->Interact(this);
	}
}

// ---------------------------------------------------------------------------------------------------------------- tick

void ASaskiaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateMovement();
	UpdateFootstepsAndHeadBob(DeltaSeconds);
	UpdateCamera(DeltaSeconds);
	UpdateFlashlight(DeltaSeconds);
	if (IsLocallyControlled())
	{
		UpdateInteractionFocus();
	}
}

void ASaskiaCharacter::UpdateMovement()
{
	// She only runs forwards, and not while crouched.
	const FVector Velocity = GetVelocity();
	const bool bForwards = FVector::DotProduct(Velocity.GetSafeNormal2D(), GetActorForwardVector()) > 0.5f;
	bSprinting = bWantsToSprint && !bIsCrouched && bForwards && Velocity.Size2D() > 50.f;
	GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;
	Breath->bIsSprinting = bSprinting;
}

void ASaskiaCharacter::UpdateFootstepsAndHeadBob(float DeltaSeconds)
{
	const float Speed = GetVelocity().Size2D();
	const bool bOnGround = GetCharacterMovement()->IsMovingOnGround();
	const int32 Gait = bSprinting ? 1 : (bIsCrouched ? 2 : 0);    // walk / run / crouch
	const float Stride = static_cast<float>(StrideLength[Gait]);

	FVector2D TargetBob = FVector2D::ZeroVector;
	if (bOnGround && Speed > 20.f)
	{
		StepDistance += Speed * DeltaSeconds;
		if (StepDistance >= Stride)
		{
			StepDistance -= Stride;
			++StepCount;

			const float Loudness = static_cast<float>(FootstepLoudness[Gait]);
			const FVector Feet = GetActorLocation() - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
			UAISense_Hearing::ReportNoiseEvent(this, Feet, Loudness, this, 0.f, TEXT("Footstep"));
			if (FootstepSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FootstepSound, Feet, 0.4f + 0.6f * Loudness);
			}
		}

		// Lowest at each footfall, highest mid-stride; a gentle sway from foot to foot.
		const float Phase = StepDistance / Stride;
		const float Amp = static_cast<float>(HeadBob[Gait]) * FMath::Clamp(Speed / WalkSpeed, 0.f, 1.5f);
		TargetBob.Y = -Amp * (0.5f + 0.5f * FMath::Cos(2.f * PI * Phase));
		TargetBob.X = 0.5f * Amp * FMath::Sin(PI * (Phase + static_cast<float>(StepCount % 2)));
	}
	else
	{
		StepDistance = 0.f;
	}
	BobOffset = FMath::Vector2DInterpTo(BobOffset, TargetBob, DeltaSeconds, 12.f);
}

void ASaskiaCharacter::UpdateCamera(float DeltaSeconds)
{
	// The camera sinks smoothly when she crouches instead of snapping down.
	CrouchEyeOffset = FMath::FInterpTo(CrouchEyeOffset, 0.f, DeltaSeconds, 9.f);

	// Breathing: a slow rise and fall, faster and deeper when she is out of breath, perfectly still while holding.
	const EBreathState State = Breath->GetBreathState();
	const bool bHolding = State == EBreathState::Holding;
	const bool bPanting = State == EBreathState::Recovering || State == EBreathState::Gasping;
	const float Rate = bPanting ? 0.75f : 0.22f;      // breaths per second
	const float Depth = bHolding ? 0.f : (bPanting ? 0.8f : 0.25f);
	BreathPhase = FMath::Fmod(BreathPhase + DeltaSeconds * Rate, 1.f);
	BreathOffset = FMath::FInterpTo(BreathOffset, Depth * FMath::Sin(2.f * PI * BreathPhase), DeltaSeconds, 6.f);

	Camera->SetRelativeLocation(FVector(0.f, BobOffset.X, EyeHeight + CrouchEyeOffset + BobOffset.Y + BreathOffset));
}

void ASaskiaCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CrouchEyeOffset += ScaledHalfHeightAdjust;   // the body just dropped: keep the eyes up, then let them sink
}

void ASaskiaCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CrouchEyeOffset -= ScaledHalfHeightAdjust;
}

// ---------------------------------------------------------------------------------------------------------------- flashlight

void ASaskiaCharacter::SetFlashlightOn(bool bOn)
{
	bFlashlightOn = bOn;
	Flashlight->SetVisibility(bOn);
}

void ASaskiaCharacter::FlickerFlashlight(float Seconds)
{
	FlickerTimeLeft = FMath::Max(FlickerTimeLeft, Seconds);
}

void ASaskiaCharacter::UpdateFlashlight(float DeltaSeconds)
{
	if (FlickerTimeLeft <= 0.f)
	{
		return;
	}
	FlickerTimeLeft -= DeltaSeconds;
	FlickerStepTimer -= DeltaSeconds;
	if (FlickerTimeLeft <= 0.f)
	{
		Flashlight->SetIntensity(FlashlightLumens);
		return;
	}
	if (FlickerStepTimer <= 0.f)
	{
		// A failing contact: mostly a weak, wavering light, sometimes nearly out.
		FlickerStepTimer = FMath::FRandRange(0.03f, 0.09f);
		const float Level = FMath::FRand() < 0.2f ? 0.03f : FMath::FRandRange(0.35f, 1.f);
		Flashlight->SetIntensity(FlashlightLumens * Level);
	}
}

// ---------------------------------------------------------------------------------------------------------------- interaction

void ASaskiaCharacter::UpdateInteractionFocus()
{
	FocusedInteractable = nullptr;

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * InteractReach;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SaskiaInteract), false, this);
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return;
	}
	if (AActor* HitActor = Hit.GetActor())
	{
		UInteractableComponent* Target = HitActor->FindComponentByClass<UInteractableComponent>();
		if (Target && Target->bEnabled)
		{
			FocusedInteractable = Target;
		}
	}
}
