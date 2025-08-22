#pragma once

#include "EnhancedInputComponent.h"
#include "GaiaInputConfig.h"
#include "GaiaInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UInputAction;
class UObject;

USTRUCT()
struct FInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	// Higher priority input mappings will be prioritized over mappings with a lower priority.
	UPROPERTY(EditAnywhere, Category="Input")
	int32 Priority = 0;
	
	/** If true, then this mapping context will be registered with the settings when this game feature action is registered. */
	UPROPERTY(EditAnywhere, Category="Input")
	bool bRegisterWithSettings = true;
};

UCLASS(Config = Input)
class UGaiaInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UGaiaInputComponent(const FObjectInitializer& ObjectInitializer);

	void AddInputMappings(const UGaiaInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const UGaiaInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template<class UserClass, typename FuncType>
	void BindNativeAction(const UGaiaInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UGaiaInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);
};

template<class UserClass, typename FuncType>
void UGaiaInputComponent::BindNativeAction(const UGaiaInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UGaiaInputComponent::BindAbilityActions(const UGaiaInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FGaiaInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}

			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}
