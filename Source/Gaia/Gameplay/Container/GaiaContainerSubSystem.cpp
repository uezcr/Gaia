#include "GaiaContainerSubSystem.h"

#include "GaiaContainerLibrary.h"
#include "GaiaItemBase.h"
#include "GaiaLogChannels.h"

bool UGaiaContainerSubSystem::RequestNewItem(FGaiaItemInfo& InOutItemInfo)
{
	FGaiaItemConfig LoadItemConfig;
	if (UGaiaContainerLibrary::GetItemConfig(InOutItemInfo.ItemName, LoadItemConfig))
	{
		InOutItemInfo.Reset();
		CurrentItemUID++;
		InOutItemInfo.ItemUID = CurrentItemUID;
		if (ItemInfos.Contains(CurrentItemUID))
		{
			ItemInfos[CurrentItemUID] = InOutItemInfo;
			UE_LOG(LogGaiaContainer,Warning,TEXT("ItemUID Already Exist, Check SaveLoadSystem"));
		}
		else
		{
			ItemInfos.Add(CurrentItemUID, InOutItemInfo);
		}
		return true;
	}
	UE_LOG(LogGaiaContainer,Warning,TEXT("RequestNewItem Failed, Invalid Item Name [%s]"), *InOutItemInfo.ItemName.ToString());
	return false;
}

bool UGaiaContainerSubSystem::UpdateItemInfo(FGaiaItemInfo& InOutItemInfo)
{
	if (ItemInfos.Contains(InOutItemInfo.ItemUID))
	{
		ItemInfos[InOutItemInfo.ItemUID] = InOutItemInfo;
		return true;
	}
	return false;
}

bool UGaiaContainerSubSystem::RequestNewContainer(FGaiaContainerInfo& InOutContainerInfo)
{
	return false;
}

bool UGaiaContainerSubSystem::UpdateContainerInfo(FGaiaContainerInfo& InOutContainerInfo)
{
	if (ContainerUIDs.Contains(InOutContainerInfo.ContainerUID))
	{
		ContainerUIDs[InOutContainerInfo.ContainerUID] = InOutContainerInfo;
		return true;
	}
	return false;
}

bool UGaiaContainerSubSystem::TransferItem(FGaiaItemInfo& InOutItemInfo, FGaiaContainerInfo& SourceContainer, FGaiaContainerInfo& TargetContainer)
{
	if (InOutItemInfo.ItemUID == INDEX_NONE) return false;
	if (SourceContainer.ContainerUID == TargetContainer.ContainerUID) return false;
	FGaiaItemConfig LoadItemConfig;
	FGaiaContainerConfig LoadContainerConfig;
	if (UGaiaContainerLibrary::GetItemConfig(InOutItemInfo.ItemName, LoadItemConfig) && UGaiaContainerLibrary::GetContainerConfig(TargetContainer.ContainerName, LoadContainerConfig))
	{
		if (LoadContainerConfig > LoadItemConfig)
		{
			SourceContainer.ItemUIDs.Remove(InOutItemInfo.ItemUID);
			UpdateContainerInfo(SourceContainer);
			TargetContainer.ItemUIDs.AddUnique(InOutItemInfo.ItemUID);
			UpdateContainerInfo(TargetContainer);
			return true;
		}
	}
	return false;
}

AGaiaItemBase* UGaiaContainerSubSystem::SpawnItem(FGaiaItemInfo& InOutItemInfo, const FTransform& InSpawnTransform)
{
	FGaiaItemConfig LoadConfig;
	if (UGaiaContainerLibrary::GetItemConfig(InOutItemInfo.ItemName, LoadConfig))
	{
		if (UClass* SpawnClass = LoadConfig.ItemClass.LoadSynchronous())
		{
			if (AGaiaItemBase* NewItem = GetWorld()->SpawnActorDeferred<AGaiaItemBase>(SpawnClass, InSpawnTransform))
			{
				NewItem->NativePreInitializeItem(InOutItemInfo);
				return NewItem;
			}
		}
	}
	return nullptr;
}


