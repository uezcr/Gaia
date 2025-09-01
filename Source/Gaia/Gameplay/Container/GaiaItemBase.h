#pragma once

#include "GaiaContainerTypes.h"
#include "GameFramework/Actor.h"
#include "GaiaItemBase.generated.h"

UCLASS(Abstract)
class GAIAGAME_API AGaiaItemBase : public AActor
{
	GENERATED_BODY()

public:
	AGaiaItemBase();

	UFUNCTION(BlueprintCallable, Category = "GaiaItemBase")
	void InitializeItem();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "GaiaItemBase|Components")
	TObjectPtr<USceneComponent> Scene;
	UPROPERTY(BlueprintReadWrite, Category = "GaiaItemBase|RuntimeParameter", DisplayName = "物品信息")
	FGaiaItemInfo ItemInfo;
	
};
