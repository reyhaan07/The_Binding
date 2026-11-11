#include "Core/BindingHUD.h"

#include "Player/SaskiaCharacter.h"
#include "Player/BreathComponent.h"
#include "Interaction/InteractableComponent.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"

void ABindingHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}
	const ASaskiaCharacter* Saskia = Cast<ASaskiaCharacter>(GetOwningPawn());
	if (!Saskia)
	{
		return;
	}

	const float Scale = Canvas->ClipY / 1080.f;
	const float CX = Canvas->ClipX * 0.5f;
	const float CY = Canvas->ClipY * 0.5f;
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;

	// What she is looking at.
	if (const UInteractableComponent* Target = Saskia->GetFocusedInteractable())
	{
		const float Dot = 4.f * Scale;
		DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.7f), CX - Dot * 0.5f, CY - Dot * 0.5f, Dot, Dot);

		const FString Text = FString::Printf(TEXT("E    %s"), *Target->Prompt.ToString());
		float W = 0.f, H = 0.f;
		GetTextSize(Text, W, H, Font, Scale);
		DrawText(Text, FLinearColor(0.86f, 0.85f, 0.8f, 0.85f), CX - W * 0.5f, CY + 30.f * Scale, Font, Scale);
	}

	// Her breath.
	if (const UBreathComponent* Breath = Saskia->GetBreath())
	{
		const float Air = Breath->GetAir();
		const bool bHolding = Breath->IsHolding();
		if (Air < 0.995f || bHolding)
		{
			const float Width = 260.f * Scale;
			const float Height = 3.f * Scale;
			const float X = CX - Width * 0.5f;
			const float Y = Canvas->ClipY - 72.f * Scale;

			FLinearColor Fill(1.f, 1.f, 1.f, 0.45f);
			if (bHolding)
			{
				Fill = FLinearColor(0.72f, 0.84f, 1.f, 0.7f);
			}
			if (Air < 0.25f)
			{
				const float Pulse = 0.5f + 0.4f * FMath::Sin(static_cast<float>(GetWorld()->GetTimeSeconds()) * 8.f);
				Fill = FLinearColor(1.f, 0.3f, 0.25f, Pulse);
			}
			DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.1f), X, Y, Width, Height);
			DrawRect(Fill, X + Width * (1.f - Air) * 0.5f, Y, Width * Air, Height);   // shrinks towards the middle
		}
	}
}
