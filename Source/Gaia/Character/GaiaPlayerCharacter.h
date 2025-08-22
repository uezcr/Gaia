#pragma once

#include "GaiaCharacter.h"
#include "Input/GaiaInputComponent.h"
#include "GaiaPlayerCharacter.generated.h"

#define UE_API GAIAGAME_API

class UGaiaInputConfig;
struct FInputActionValue;

UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class AGaiaPlayerCharacter : public AGaiaCharacter
{
	GENERATED_BODY()

public:
	UE_API AGaiaPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UE_API virtual void BeginPlay() override;

	UE_API virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	UE_API void Input_Move(const FInputActionValue& InputActionValue);
	UE_API void Input_LookMouse(const FInputActionValue& InputActionValue);

public:
	UE_API virtual void Tick(float DeltaTime) override;

	UE_API virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gaia|Input")
	TObjectPtr<UGaiaInputConfig> DefaultInputConfig;
	UPROPERTY(EditDefaultsOnly, Category = "Gaia|Input")
	FInputMappingContextAndPriority DefaultInputMapping;
};
