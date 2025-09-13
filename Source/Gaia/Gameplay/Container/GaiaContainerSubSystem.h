#pragma once

#include "GaiaContainerTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GaiaContainerSubSystem.generated.h"

#define UE_API GAIAGAME_API

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Gaia Container Manager"), MinimalAPI)
class UGaiaContainerManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGaiaContainerManagerSettings(){}
	UPROPERTY(config, EditAnywhere, Category = "Data Registry")
	FName ItemDefaultRegistryType;
	UPROPERTY(config, EditAnywhere, Category = "Data Registry")
	FName ContainerDefaultRegistryType;
};

UCLASS(MinimalAPI)
class UGaiaContainerSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~BEGIN STATIC FUNCTIONS
	UFUNCTION(BlueprintPure, Category = "Gaia|Container")
	static bool GetItemDefaultByID(FName InItemID, FGaiaItemDefault& OutItemDefault);
	UFUNCTION(BlueprintPure, Category = "Gaia|Container")
	static bool GetContainerDefaultByID(FName InContainerID, FGaiaContainerDefault& OutContainerDefault);
	UFUNCTION(BlueprintPure, Category = "Gaia|Container")
	static int32 GetItemIndexBySlotID(TArray<FGaiaItemInfo>& InItems, int32 InSlotID);
	//~END STATIC FUNCTIONS

	//~BEGIN MAIN FUNCTIONS
	UFUNCTION(BlueprintCallable, Category = "Gaia|Container")
	bool RequestNewContainer(FName InContainerID, FGaiaContainerInfo& OutContainerInfo);
	UFUNCTION(BlueprintCallable, Category = "Gaia|Container")
	bool MoveItem(int32 InSourceContainerUID, const FGaiaSlotAddress& InSourceSlotAddress, int32 InTargetContainerUID, const FGaiaSlotAddress& InTargetSlotAddress, int32 InAmount);
	bool RepositionItem(const FGaiaSlotAddress& InSourceSlotAddress, const FGaiaSlotAddress& InTargetSlotAddress, const int32 InAmount);
	bool GetItemInfo(const FGaiaSlotAddress& InSlotAddress, FGaiaItemInfo& OutItemInfo);
	bool CanDropToSlot(const FGaiaItemInfo& InItemToDrop,const FGaiaSlotAddress& InTargetAddress) const;
	bool CanAddInside(const FGaiaItemInfo& InItemToAdd, const FGaiaSlotAddress& InParentSlotAddress, FGaiaSlotAddress& OutAddressToAdd, int32& OutRemainder);
	bool ValidSpaceOnContainer(const int32 InContainerUID, const FName InItemID, const int32 InAmount,int32& OutFoundSlotID,int32& OutRemainder);
	bool TryToStack(const FGaiaSlotAddress& InTargetItemAddress, const FName InItemID, const int32 InItemAmount, int32& OutRemainder);
	//~END MAIN FUNCTIONS
public:
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|Container")
	TMap<int32, FGaiaContainerInfo> Containers;

private:
	int32 CurrentContainerUID = INDEX_NONE;
};
#undef UE_API