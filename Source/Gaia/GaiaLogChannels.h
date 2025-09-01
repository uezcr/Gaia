#pragma once

#include "Logging/LogMacros.h"

class UObject;

GAIAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogGaia, Log, All);
GAIAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogGaiaExperience, Log, All);
GAIAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogGaiaAbilitySystem, Log, All);
GAIAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogGaiaContainer, Log, All);

GAIAGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);