#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BindingHUD.generated.h"

/**
 * A deliberately quiet HUD, drawn in code so it needs no widget assets:
 *  - a small dot and a prompt ("E  Switch off the lamp") only while Saskia looks at something usable;
 *  - a thin breath line at the bottom only while her lungs are not full (pale blue while holding, red when nearly
 *    empty).
 * To hide it (cutscenes), call Show HUD (false) on the player controller's HUD.
 * Replace with a UMG widget later if you want styled art; the data comes from SaskiaCharacter / BreathComponent.
 */
UCLASS()
class THEBINDING_API ABindingHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
