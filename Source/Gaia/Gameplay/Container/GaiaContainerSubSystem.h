#pragma once

#include "CoreMinimal.h"
#include "GaiaContainerTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GaiaContainerSubSystem.generated.h"

UCLASS()
class GAIAGAME_API UGaiaContainerSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	bool RequestNewItem(const FName& InContainerName, FGaiaItemInfo& InOutItemInfo);
	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	bool UpdateItemInfo(FGaiaItemInfo& InOutItemInfo);
	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	bool RequestNewContainer(const FName& InContainerName, FGaiaContainerInfo& InOutContainerInfo);
	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	bool UpdateContainerInfo(FGaiaContainerInfo& InOutContainerInfo);

	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	bool GetItemInfo(const int64& InItemUID, FGaiaItemInfo& OutItemInfo);
	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	bool GetContainerInfo(const int64& InContainerUID, FGaiaContainerInfo& OutContainerInfo);

	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	void RemoveItemFromContainer(FGaiaContainerInfo& SourceContainer, const FGaiaItemInfo& ItemToRemove);
	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	void AddItemToContainer(const int64& TargetContainerUID, const int64& ItemUID);


	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	bool TransferItem(FGaiaItemInfo& InOutItemInfo, FGaiaContainerInfo& SourceContainer, FGaiaContainerInfo& TargetContainer);


	
	UFUNCTION(BlueprintCallable,Category = "GaiaContainerSubSystem")
	AGaiaItemBase* SpawnItem(FGaiaItemInfo& InOutItemInfo, const FTransform& InSpawnTransform);
	

private:
	UPROPERTY(BlueprintReadOnly, Category = "GaiaContainerSubSystem", meta=(AllowPrivateAccess=true))
	int64 CurrentItemUID = INDEX_NONE;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaContainerSubSystem", meta=(AllowPrivateAccess=true))
	int64 CurrentContainerUID = INDEX_NONE;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaContainerSubSystem", meta=(AllowPrivateAccess=true))
	TMap<int64, FGaiaItemInfo> ItemInfos;
	UPROPERTY(BlueprintReadOnly, Category = "GaiaContainerSubSystem", meta=(AllowPrivateAccess=true))
	TMap<int64, FGaiaContainerInfo> Containers;
};
