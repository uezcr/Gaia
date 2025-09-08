#pragma once

#include "UI/GaiaUserWidget.h"
#include "GaiaContainerSlotWidget.generated.h"

#define UE_API GAIAGAME_API

class UGaiaItemSlotWidget;
class UUniformGridPanel;

UCLASS(Abstract)
class GAIAGAME_API UGaiaContainerSlotWidget : public UGaiaUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "GaiaContainerSlotWidget")
	void SetContainerUID(const int64& InContainerUID);

protected:
	UFUNCTION(BlueprintCallable, Category = "GaiaContainerSlotWidget")
	void CreateItemSlots() const;
	
protected:
	UPROPERTY(BlueprintReadWrite, Category = "GaiaContainerSlotWidget", meta=(BindWidget = true))
	TObjectPtr<UUniformGridPanel> GridPanel;
	UPROPERTY(EditDefaultsOnly, Category = "GaiaContainerSlotWidget")
	TSubclassOf<UGaiaItemSlotWidget> ItemSlotClass;
	UPROPERTY(BlueprintReadOnly)
	int64 ContainerUID;
};
#undef UE_API