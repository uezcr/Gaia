#pragma once

#include "Logging/LogMacros.h"

#define GAIA_LOG_INFO(fmt, ...) \
UE_LOG(LogGaia, Log, TEXT(fmt), ##__VA_ARGS__)

#define GAIA_LOG_WARNING(fmt, ...) \
UE_LOG(LogGaia, Warning, TEXT(fmt), ##__VA_ARGS__)

#define GAIA_LOG_ERROR(fmt, ...) \
UE_LOG(LogGaia, Error, TEXT(fmt), ##__VA_ARGS__)

class UObject;

GAIAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogGaia, Log, All);
GAIAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogGaiaExperience, Log, All);
GAIAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogGaiaAbilitySystem, Log, All);
GAIAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogGaiaContainer, Log, All);

GAIAGAME_API FString GetClientServerContextString(UObject* ContextObject = nullptr);