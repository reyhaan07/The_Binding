#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractSignature, AActor*, Interactor);

/**
 * Makes an actor usable by Saskia (look at it, press E).
 *
 * In C++: bind OnInteract in BeginPlay.
 * In a Blueprint: add this component, set Prompt, then click the green "+" next to On Interact.
 *
 * The actor needs something that blocks the Visibility trace (a mesh with collision) so Saskia's view can find it.
 */
UCLASS(ClassGroup = (Binding), meta = (BlueprintSpawnableComponent))
class THEBINDING_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	/** What the HUD shows when Saskia looks at this, e.g. "Answer the phone". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText Prompt;

	/** Turn off to make the object ignore Saskia (no prompt, no use). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bEnabled = true;

	/** Fired when Saskia uses this object. */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractSignature OnInteract;

	/** Uses the object (does nothing while disabled). */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
};
