#pragma once

#include "CoreMinimal.h"
#include "GaiaInteractionTypes.h"
#include "UObject/Interface.h"
#include "GaiaInteractableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UGaiaInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class GAIAGAME_API IGaiaInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GaiaInteractable")
	void Selected();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GaiaInteractable")
	void EndSelected();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GaiaInteractable")
	void Interacted(UPARAM(ref) FGaiaInteractionContext& Context);
};
