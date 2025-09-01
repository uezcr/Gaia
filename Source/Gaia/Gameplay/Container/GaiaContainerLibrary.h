#pragma once

#include "GaiaContainerTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GaiaContainerLibrary.generated.h"

#define UE_API GAIAGAME_API

UCLASS(MinimalAPI)
class GAIAGAME_API UGaiaContainerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Get")
	static UE_API bool GetItemConfigByItemId(const FName InItemId, FGaiaItemConfig& OutItemConfig);
};
#undef UE_API