#pragma once

#include "CommonLocalPlayer.h"
#include "GaiaLocalPlayer.generated.h"

#define UE_API GAIAGAME_API

class UGaiaSettingsShared;

UCLASS(MinimalAPI)
class UGaiaLocalPlayer : public UCommonLocalPlayer
{
	GENERATED_BODY()

public:
	UFUNCTION()
	UE_API UGaiaSettingsShared* GetSharedSettings() const;

private:
	UPROPERTY(Transient)
	mutable TObjectPtr<UGaiaSettingsShared> SharedSettings;
};
