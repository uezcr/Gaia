#pragma once

#include "GaiaContainerTypes.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Interaction/GaiaInteractableInterface.h"
#include "GaiaItemBase.generated.h"

#define UE_API GAIAGAME_API

UCLASS(Abstract)
class AGaiaItemBase : public AActor, public IGaiaInteractableInterface
{
	GENERATED_BODY()

public:
	AGaiaItemBase();
	
	UE_API virtual void NativePreInitializeItem(const FGaiaItemInfo& InItemInfo = FGaiaItemInfo());
	UE_API virtual void NativeInitializeItem(const FGaiaItemConfig& InItemConfig);
	UE_API virtual void NativePostInitializeItem(const FGaiaItemConfig& InItemConfig);
	UFUNCTION(BlueprintImplementableEvent, Category = "GaiaItemBase")
	UE_API void PreInitializeItem(const FGaiaItemInfo& InItemInfo = FGaiaItemInfo());
	UFUNCTION(BlueprintImplementableEvent, Category = "GaiaItemBase")
	UE_API void InitializeItem(const FGaiaItemConfig& InItemConfig);
	UFUNCTION(BlueprintImplementableEvent, Category = "GaiaItemBase")
	UE_API void PostInitializeItem(const FGaiaItemConfig& InItemConfig);

	//~Begin IGaiaInteractableInterface
	virtual void Selected_Implementation() override;
	virtual void EndSelected_Implementation() override;
	virtual void Interacted_Implementation(FGaiaInteractionContext& Context) override;
	//~End IGaiaInteractableInterface
	
	UFUNCTION(BlueprintCallable, Category = "GaiaItemBase")
	UE_API void SetItemInfo(const FGaiaItemInfo& InItemInfo) {ItemInfo = InItemInfo;}

protected:
	UE_API virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaItemBase|Components")
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(BlueprintReadWrite, Category = "GaiaItemBase|RuntimeParameter", DisplayName = "物品信息")
	FGaiaItemInfo ItemInfo;
	UPROPERTY(BlueprintReadWrite, Category = "GaiaItemBase|RuntimeParameter", DisplayName = "是否可选中")
	bool bCanSelected = false;
	UPROPERTY(BlueprintReadWrite, Category = "GaiaItemBase|RuntimeParameter", DisplayName = "是否可交互")
	bool bCanInteracted = false;
};
#undef UE_API
