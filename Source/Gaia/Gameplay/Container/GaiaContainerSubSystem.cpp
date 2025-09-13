#include "GaiaContainerSubSystem.h"

#include "DataRegistrySubsystem.h"
#include "GaiaLogChannels.h"

bool UGaiaContainerSubSystem::GetItemDefaultByID(const FName InItemID, FGaiaItemDefault& OutItemDefault)
{
	if (const UGaiaContainerManagerSettings* ContainerManagerSettings = GetDefault<UGaiaContainerManagerSettings>())
	{
		if (InItemID != NAME_None)
		{
			if (UDataRegistrySubsystem * DataRegistrySubsystem = GEngine->GetEngineSubsystem<UDataRegistrySubsystem>())
			{
				OutItemDefault = *DataRegistrySubsystem->GetCachedItem<FGaiaItemDefault>(FDataRegistryId(ContainerManagerSettings->ItemDefaultRegistryType,InItemID));
				return true;
			}
		}
	}
	return false;
}

bool UGaiaContainerSubSystem::GetContainerDefaultByID(const FName InContainerID, FGaiaContainerDefault& OutContainerDefault)
{
	if (const UGaiaContainerManagerSettings* ContainerManagerSettings = GetDefault<UGaiaContainerManagerSettings>())
	{
		if (InContainerID != NAME_None)
		{
			if (UDataRegistrySubsystem * DataRegistrySubsystem = GEngine->GetEngineSubsystem<UDataRegistrySubsystem>())
			{
				OutContainerDefault = *DataRegistrySubsystem->GetCachedItem<FGaiaContainerDefault>(FDataRegistryId(ContainerManagerSettings->ContainerDefaultRegistryType,InContainerID));
				return true;
			}
		}
	}
	return false;
}

int32 UGaiaContainerSubSystem::GetItemIndexBySlotID(TArray<FGaiaItemInfo>& InItems, const int32 InSlotID)
{
	for (int32 i = 0; i < InItems.Num(); i++)
	{
		if (InItems[i].ItemAddress.SlotID == InSlotID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool UGaiaContainerSubSystem::MoveItem(int32 InSourceContainerUID, const FGaiaSlotAddress& InSourceSlotAddress,
                                       int32 InTargetContainerUID, const FGaiaSlotAddress& InTargetSlotAddress, int32 InAmount)
{
	if (!Containers.Contains(InSourceContainerUID) || !Containers.Contains(InTargetContainerUID)) return false;
	if (InSourceContainerUID == InTargetContainerUID)
	{
		Containers[InSourceContainerUID].ContainerUID = InTargetContainerUID;
		return false;
	}
	return false;
}

bool UGaiaContainerSubSystem::RepositionItem(const FGaiaSlotAddress& InSourceSlotAddress, const FGaiaSlotAddress& InTargetSlotAddress, const int32 InAmount)
{
	if (InSourceSlotAddress.ContainerUID != InTargetSlotAddress.ContainerUID)
	{
		UE_LOG(LogGaiaContainer, Error, TEXT("UGaiaContainerSubSystem::RepositionItem 执行失败,物品必须在同一容器中."));
		return false;
	}
	if (InSourceSlotAddress == InTargetSlotAddress)
	{
		UE_LOG(LogGaiaContainer, Warning, TEXT("UGaiaContainerSubSystem::RepositionItem 执行警告,物品地址相同."));
		return false;
	}
	const int32 ContainerUID = InSourceSlotAddress.ContainerUID;
	if (Containers.Contains(ContainerUID))
	{
		FGaiaItemInfo SourceItemInfoCopy;
		GetItemInfo(InSourceSlotAddress,SourceItemInfoCopy);
		FGaiaItemInfo ItemToReposition = SourceItemInfoCopy;
		ItemToReposition.ItemAmount = FMath::Clamp(InAmount,0,SourceItemInfoCopy.ItemAmount);
		
	}
	return false;
}

bool UGaiaContainerSubSystem::GetItemInfo(const FGaiaSlotAddress& InSlotAddress, FGaiaItemInfo& OutItemInfo)
{
	const int32 ContainerUID = InSlotAddress.ContainerUID;
	if (Containers.Contains(ContainerUID))
	{
		for (int32 i = 0; i < Containers[ContainerUID].Items.Num(); i++)
		{
			if (Containers[ContainerUID].Items[i].ItemAddress.SlotID == InSlotAddress.SlotID)
			{
				OutItemInfo = Containers[ContainerUID].Items[i];
				return true;
			}
		}
	}
	return false;
}

bool UGaiaContainerSubSystem::CanDropToSlot(const FGaiaItemInfo& InItemToDrop, const FGaiaSlotAddress& InTargetAddress) const
{
	if (!Containers.Contains(InTargetAddress.ContainerUID))
	{
		UE_LOG(LogGaiaContainer, Error, TEXT("UGaiaContainerSubSystem::CanDropToSlot 执行失败,容器UID无效."));
		return false;
	}
	return false;
}

bool UGaiaContainerSubSystem::CanAddInside(const FGaiaItemInfo& InItemToAdd, const FGaiaSlotAddress& InParentSlotAddress, FGaiaSlotAddress& OutAddressToAdd, int32& OutRemainder)
{
	OutAddressToAdd = FGaiaSlotAddress();
	OutRemainder = 0;
	FGaiaItemInfo ParentItemInfoCopy;
	if (!GetItemInfo(InParentSlotAddress,ParentItemInfoCopy)) return false;
	if (ParentItemInfoCopy.ItemAddress == InItemToAdd.ItemAddress) return false;
	if (ParentItemInfoCopy.OwnContainerUID == INDEX_NONE) return false;
	if (InItemToAdd.ItemAddress.ContainerUID == ParentItemInfoCopy.OwnContainerUID) return false;
	FGaiaItemDefault ItemToAddDefault;
	if (!GetItemDefaultByID(InItemToAdd.ItemID,ItemToAddDefault)) return false;
	return false;
}

bool UGaiaContainerSubSystem::ValidSpaceOnContainer(const int32 InContainerUID, const FName InItemID, const int32 InAmount, int32& OutFoundSlotID, int32& OutRemainder)
{
	OutFoundSlotID = INDEX_NONE;
	OutRemainder = 0;
	if (!Containers.Contains(InContainerUID)) return false;
	FGaiaItemDefault ItemToAddDefault;
	if (!GetItemDefaultByID(InItemID,ItemToAddDefault)) return false;
	FGaiaContainerDefault ContainerDefault;
	if (!GetContainerDefaultByID(Containers[InContainerUID].ContainerID,ContainerDefault)) return false;
	if (!ContainerDefault.HasTag(ItemToAddDefault.ItemTag)) return false;
	// 是否可以叠加到相同道具.
	for (const FGaiaItemInfo& ItemInfo : Containers[InContainerUID].Items)
	{
		int Remainder = 0;
		if (TryToStack(ItemInfo.ItemAddress,InItemID,InAmount,Remainder))
		{
			OutFoundSlotID = ItemInfo.ItemAddress.SlotID;
			OutRemainder = Remainder;
			return true;
		}
	}
	// 没有可叠加的道具,找空位置.
	return false;
}

bool UGaiaContainerSubSystem::TryToStack(const FGaiaSlotAddress& InTargetItemAddress, const FName InItemID, const int32 InItemAmount, int32& OutRemainder)
{
	OutRemainder = 0;
	FGaiaItemInfo ItemInfoCopy;
	if (!GetItemInfo(InTargetItemAddress,ItemInfoCopy)) return false;
	if (ItemInfoCopy.ItemID != InItemID) return false;
	FGaiaItemDefault ItemDefault;
	if (!GetItemDefaultByID(InItemID,ItemDefault)) return false;
	if (!ItemDefault.bStackable) return false;
	int32 RemainderAmount = ItemInfoCopy.ItemAmount + InItemAmount - FMath::Clamp(ItemInfoCopy.ItemAmount + InItemAmount,0,ItemDefault.MaxStackSize);
	if (!RemainderAmount < InItemAmount) return false;
	OutRemainder = RemainderAmount;
	return true;
}

bool UGaiaContainerSubSystem::RequestNewContainer(const FName InContainerID, FGaiaContainerInfo& OutContainerInfo)
{
	FGaiaContainerDefault ContainerDefault;
	if (GetContainerDefaultByID(InContainerID, ContainerDefault))
	{
		OutContainerInfo.ContainerUID = ++CurrentContainerUID;
		OutContainerInfo.ContainerID = InContainerID;
		Containers.Add(CurrentContainerUID, OutContainerInfo);
		return true;
	}
	return false;
}
