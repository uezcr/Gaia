#pragma once

#include "ModularCharacter.h"
#include "GaiaCharacter.generated.h"

#define UE_API GAIAGAME_API

UCLASS(Const, MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class AGaiaCharacter : public AModularCharacter
{
	GENERATED_BODY()

public:
	UE_API AGaiaCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UE_API virtual void BeginPlay() override;

public:
	UE_API virtual void Tick(float DeltaTime) override;

	UE_API virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
