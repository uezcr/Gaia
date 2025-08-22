#include "GaiaInputConfig.h"
#include "GaiaLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GaiaInputConfig)

UGaiaInputConfig::UGaiaInputConfig(const FObjectInitializer& ObjectInitializer)
{
}

const UInputAction* UGaiaInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FGaiaInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogGaia, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}

const UInputAction* UGaiaInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FGaiaInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogGaia, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
