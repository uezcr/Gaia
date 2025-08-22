#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GaiaInputConfig.generated.h"

class UInputAction;
class UObject;
struct FFrame;

USTRUCT(BlueprintType)
struct FGaiaInputAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

UCLASS(BlueprintType, Const)
class UGaiaInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UGaiaInputConfig(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Gaia|Pawn")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	UFUNCTION(BlueprintCallable, Category = "Gaia|Pawn")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FGaiaInputAction> NativeInputActions;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputAction"))
	TArray<FGaiaInputAction> AbilityInputActions;
};
