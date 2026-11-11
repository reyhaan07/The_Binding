#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BindingLamp.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UInteractableComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLampLitChanged, bool, bIsLit);

/**
 * A lamp Saskia can switch on and off (E). The first object the game teaches with: "Turn your lamp off" (Story Bible
 * 4.4). Placeholder meshes from the engine's basic shapes; swap them for the real lamp in a Blueprint child.
 *
 * SetListening is the Bound One's tell (Story Bible 7.2): the nearest linked lamp slowly dims and wavers, as if
 * something is listening.
 *
 * OnLitChanged is what the link system will listen to later, to copy on/off across a tag.
 */
UCLASS()
class THEBINDING_API ABindingLamp : public AActor
{
	GENERATED_BODY()

public:
	ABindingLamp();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Lamp")
	void SetLit(bool bNewLit);

	UFUNCTION(BlueprintCallable, Category = "Lamp")
	void Toggle() { SetLit(!bLit); }

	UFUNCTION(BlueprintPure, Category = "Lamp")
	bool IsLit() const { return bLit; }

	/** 0 = nothing near, 1 = the Bound One is right here. Dims the lamp and makes it waver. */
	UFUNCTION(BlueprintCallable, Category = "Lamp")
	void SetListening(float Amount);

	UPROPERTY(BlueprintAssignable, Category = "Lamp")
	FOnLampLitChanged OnLitChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lamp")
	TObjectPtr<UStaticMeshComponent> Base;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lamp")
	TObjectPtr<UStaticMeshComponent> Shade;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lamp")
	TObjectPtr<UPointLightComponent> Light;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lamp")
	TObjectPtr<UInteractableComponent> Interactable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp")
	bool bLit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp")
	float LitLumens = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp")
	TObjectPtr<USoundBase> SwitchSound;

private:
	UFUNCTION()
	void HandleInteract(AActor* Interactor);

	void ApplyLight();
	void UpdatePrompt();

	float Listening = 0.f;
	float Waver = 1.f;
	float WaverTimer = 0.f;
};
