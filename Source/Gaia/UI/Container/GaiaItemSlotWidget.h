#pragma once

#include "Gameplay/Container/GaiaContainerTypes.h"
#include "UI/GaiaUserWidget.h"
#include "GaiaItemSlotWidget.generated.h"

#define UE_API GAIAGAME_API

class UCommonTextBlock;

UCLASS(Abstract)
class UGaiaItemSlotWidget : public UGaiaUserWidget
{
	GENERATED_BODY()

public:
	void InitItemSlot(const int64& InItemUID);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "GaiaItemSlotWidget")
	int64 ItemUID;
};

#undef UE_API
