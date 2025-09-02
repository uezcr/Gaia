#pragma once

#include "GaiaContainerTypes.h"
#include "GameFramework/Actor.h"
#include "GaiaItemBase.generated.h"

#define UE_API GAIAGAME_API

UCLASS(Abstract)
class AGaiaItemBase : public AActor
{
	GENERATED_BODY()

public:
	AGaiaItemBase();
	
	UE_API virtual void NativePreInitializeItem(const FGaiaItemInfo& InItemInfo = FGaiaItemInfo());
	UE_API virtual void NativeInitializeItem(const FGaiaItemConfig& InItemConfig);
	UE_API virtual void NativePostInitializeItem();
	UFUNCTION(BlueprintImplementableEvent, Category = "GaiaItemBase")
	UE_API void PreInitializeItem(const FGaiaItemInfo& InItemInfo = FGaiaItemInfo());
	UFUNCTION(BlueprintImplementableEvent, Category = "GaiaItemBase")
	UE_API void InitializeItem(const FGaiaItemConfig& InItemConfig);
	UFUNCTION(BlueprintImplementableEvent, Category = "GaiaItemBase")
	UE_API void PostInitializeItem();
	
	UFUNCTION(BlueprintCallable, Category = "GaiaItemBase")
	UE_API void SetItemInfo(const FGaiaItemInfo& InItemInfo) {ItemInfo = InItemInfo;}

protected:
	UE_API virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaItemBase|Components")
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(BlueprintReadWrite, Category = "GaiaItemBase|RuntimeParameter", DisplayName = "物品信息")
	FGaiaItemInfo ItemInfo;
};
#undef UE_API
