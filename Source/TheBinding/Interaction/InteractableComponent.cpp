#include "Interaction/InteractableComponent.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Prompt = NSLOCTEXT("Binding", "DefaultPrompt", "Use");
}

void UInteractableComponent::Interact(AActor* Interactor)
{
	if (bEnabled)
	{
		OnInteract.Broadcast(Interactor);
	}
}
