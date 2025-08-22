#pragma once

#include "ModularGameMode.h"
#include "GaiaGameMode.generated.h"

#define UE_API GAIAGAME_API

UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class AGaiaGameMode : public AModularGameModeBase
{
	GENERATED_BODY()

public:
	UE_API AGaiaGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
