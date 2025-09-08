#include "GaiaContainerSubSystem.h"

#include "GaiaContainerLibrary.h"
#include "GaiaItemBase.h"
#include "GaiaLogChannels.h"

bool UGaiaContainerSubSystem::RequestNewItem(const FName& InContainerName, FGaiaItemInfo& InOutItemInfo)
{
	FGaiaItemConfig LoadItemConfig;
	if (UGaiaContainerLibrary::GetItemConfig(InContainerName, LoadItemConfig))
	{
		InOutItemInfo = FGaiaItemInfo();
		InOutItemInfo.ItemName = InContainerName;
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

bool UGaiaContainerSubSystem::RequestNewContainer(const FName& InContainerName, FGaiaContainerInfo& InOutContainerInfo)
{
	FGaiaContainerConfig LoadContainerConfig;
	if (UGaiaContainerLibrary::GetContainerConfig(InContainerName, LoadContainerConfig))
	{
		InOutContainerInfo = FGaiaContainerInfo();
		InOutContainerInfo.ContainerName = InContainerName;
		CurrentContainerUID++;
		InOutContainerInfo.ContainerUID = CurrentContainerUID;
		int32 GridNum = LoadContainerConfig.GridNums.X * LoadContainerConfig.GridNums.Y;
		for (int32 i = 0; i < GridNum; i++)
		{
			InOutContainerInfo.Items.Add(i,INDEX_NONE);
		}
		if (Containers.Contains(CurrentContainerUID))
		{
			Containers[CurrentContainerUID] = InOutContainerInfo;
			UE_LOG(LogGaiaContainer,Warning,TEXT("ContainerUID Already Exist, Check SaveLoadSystem"));
		}
		else
		{
			Containers.Add(CurrentContainerUID, InOutContainerInfo);
		}
	}
	return false;
}

bool UGaiaContainerSubSystem::UpdateContainerInfo(FGaiaContainerInfo& InOutContainerInfo)
{
	if (Containers.Contains(InOutContainerInfo.ContainerUID))
	{
		Containers[InOutContainerInfo.ContainerUID] = InOutContainerInfo;
		return true;
	}
	return false;
}

bool UGaiaContainerSubSystem::GetItemInfo(const int64& InItemUID, FGaiaItemInfo& OutItemInfo)
{
	if (ItemInfos.Contains(InItemUID))
	{
		OutItemInfo = ItemInfos[InItemUID];
		return true;
	}
	return false;
}

bool UGaiaContainerSubSystem::GetContainerInfo(const int64& InContainerUID, FGaiaContainerInfo& OutContainerInfo)
{
	if (Containers.Contains(InContainerUID))
	{
		OutContainerInfo = Containers[InContainerUID];
		return true;
	}
	return false;
}

void UGaiaContainerSubSystem::RemoveItemFromContainer(FGaiaContainerInfo& SourceContainer, const FGaiaItemInfo& ItemToRemove)
{
	for (TPair<int32, int64>& Slot : SourceContainer.Items)
	{
		if (Slot.Value == ItemToRemove.ItemUID)
		{
			Slot.Value = INDEX_NONE;
		}
	}
}

void UGaiaContainerSubSystem::AddItemToContainer(const int64& TargetContainerUID, const int64& ItemUID)
{
	if (Containers.Contains(TargetContainerUID))
	{
		for (TPair<int32, int64>& Slot : Containers[TargetContainerUID].Items)
		{
			if (Slot.Value == INDEX_NONE)
			{
				Slot.Value = ItemUID;
				break;
			}
		}
	}
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
			RemoveItemFromContainer(SourceContainer, InOutItemInfo);
			UpdateContainerInfo(SourceContainer);
			AddItemToContainer(TargetContainer.ContainerUID, InOutItemInfo.ItemUID);
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


