#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BindingGameMode.generated.h"

/**
 * The game's rules for a level: spawns Saskia and the HUD.
 * Make a Blueprint child (BP_BindingGameMode) and set its Default Pawn Class to your BP_Saskia (see README).
 */
UCLASS()
class THEBINDING_API ABindingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABindingGameMode();
};
