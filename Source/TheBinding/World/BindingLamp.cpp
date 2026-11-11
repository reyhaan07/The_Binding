#include "World/BindingLamp.h"

#include "Interaction/InteractableComponent.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ABindingLamp::ABindingLamp()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));

	// Pivot on the floor (or table) under the lamp. The engine's basic shapes are 100 cm across, pivot in the middle.
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	Base->SetupAttachment(Root);
	Base->SetStaticMesh(CylinderMesh.Object);
	Base->SetRelativeLocation(FVector(0.f, 0.f, 22.5f));
	Base->SetRelativeScale3D(FVector(0.12f, 0.12f, 0.45f));        // a 45 cm stem

	Shade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shade"));
	Shade->SetupAttachment(Root);
	Shade->SetStaticMesh(ConeMesh.Object);
	Shade->SetRelativeLocation(FVector(0.f, 0.f, 52.f));
	Shade->SetRelativeScale3D(FVector(0.34f, 0.34f, 0.24f));
	Shade->SetCastShadow(false);                                   // lets the bulb's light out through the placeholder shade

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Root);
	Light->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	Light->IntensityUnits = ELightUnits::Lumens;
	Light->Intensity = LitLumens;
	Light->AttenuationRadius = 1000.f;
	Light->SourceRadius = 4.f;
	Light->bUseTemperature = true;
	Light->Temperature = 2700.f;                       // a warm old bulb

	Interactable = CreateDefaultSubobject<UInteractableComponent>(TEXT("Interactable"));
}

void ABindingLamp::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyLight();
	UpdatePrompt();
}

void ABindingLamp::BeginPlay()
{
	Super::BeginPlay();
	Interactable->OnInteract.AddDynamic(this, &ABindingLamp::HandleInteract);
	ApplyLight();
	UpdatePrompt();
}

void ABindingLamp::HandleInteract(AActor* Interactor)
{
	Toggle();
}

void ABindingLamp::SetLit(bool bNewLit)
{
	if (bLit == bNewLit)
	{
		return;
	}
	bLit = bNewLit;
	ApplyLight();
	UpdatePrompt();
	if (SwitchSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SwitchSound, GetActorLocation());
	}
	OnLitChanged.Broadcast(bLit);
}

void ABindingLamp::SetListening(float Amount)
{
	Listening = FMath::Clamp(Amount, 0.f, 1.f);
	if (Listening <= 0.f)
	{
		Waver = 1.f;
	}
	ApplyLight();
}

void ABindingLamp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bLit || Listening <= 0.f)
	{
		return;
	}
	WaverTimer -= DeltaSeconds;
	if (WaverTimer <= 0.f)
	{
		// The closer it is, the more the filament struggles.
		WaverTimer = FMath::FRandRange(0.05f, 0.18f);
		Waver = 1.f - Listening * FMath::FRandRange(0.f, 0.35f);
		ApplyLight();
	}
}

void ABindingLamp::ApplyLight()
{
	const float Level = bLit ? (1.f - 0.8f * Listening) * Waver : 0.f;
	Light->SetIntensity(LitLumens * Level);
}

void ABindingLamp::UpdatePrompt()
{
	Interactable->Prompt = bLit ? NSLOCTEXT("Binding", "LampOff", "Switch off the lamp")
	                            : NSLOCTEXT("Binding", "LampOn", "Switch on the lamp");
}
