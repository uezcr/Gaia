#include "GaiaInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Player/GaiaLocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GaiaInputComponent)

class UGameInputConfig;

UGaiaInputComponent::UGaiaInputComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UGaiaInputComponent::AddInputMappings(const UGaiaInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UGaiaInputComponent::RemoveInputMappings(const UGaiaInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UGaiaInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for (uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);
	}
	BindHandles.Reset();
}

