#include "Core/BindingGameMode.h"

#include "Core/BindingHUD.h"
#include "Player/SaskiaCharacter.h"

ABindingGameMode::ABindingGameMode()
{
	DefaultPawnClass = ASaskiaCharacter::StaticClass();
	HUDClass = ABindingHUD::StaticClass();
}
