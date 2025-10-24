#include "GaiaInventorySubsystem.h"
#include "GaiaLogChannels.h"
#include "DataRegistrySubsystem.h"

UGaiaInventorySubsystem::UGaiaInventorySubsystem()
{
}

void UGaiaInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogGaia, Log, TEXT("GaiaInventorySubsystem 初始化"));
	
	// 清空全局物品池和容器映射表
	AllItems.Empty();
	Containers.Empty();
}

void UGaiaInventorySubsystem::Deinitialize()
{
	UE_LOG(LogGaia, Log, TEXT("GaiaInventorySubsystem 反初始化"));
	
	// 清理所有物品和容器
	AllItems.Empty();
	Containers.Empty();
	
	Super::Deinitialize();
}

UGaiaInventorySubsystem* UGaiaInventorySubsystem::Get(const UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			return World->GetSubsystem<UGaiaInventorySubsystem>();
		}
	}
	return nullptr;
}

//~BEGIN 数据定义获取

bool UGaiaInventorySubsystem::GetItemDefinition(FName ItemDefID, FGaiaItemDefinition& OutItemDef)
{
	const UGaiaInventoryManagerSettings* Settings = GetDefault<UGaiaInventoryManagerSettings>();
	if (!Settings)
	{
		UE_LOG(LogGaia, Error, TEXT("无法获取GaiaInventoryManagerSettings"));
		return false;
	}
	
	if (ItemDefID == NAME_None)
	{
		UE_LOG(LogGaia, Warning, TEXT("ItemDefID为空"));
		return false;
	}
	
	if (UDataRegistrySubsystem* DataRegistry = GEngine->GetEngineSubsystem<UDataRegistrySubsystem>())
	{
		const FGaiaItemDefinition* ItemDef = DataRegistry->GetCachedItem<FGaiaItemDefinition>(
			FDataRegistryId(Settings->ItemDefinitionRegistryType, ItemDefID));
		
		if (ItemDef)
		{
			OutItemDef = *ItemDef;
			return true;
		}
	}
	
	UE_LOG(LogGaia, Warning, TEXT("无法找到物品定义: %s"), *ItemDefID.ToString());
	return false;
}

bool UGaiaInventorySubsystem::GetContainerDefinition(FName ContainerDefID, FGaiaContainerDefinition& OutContainerDef)
{
	const UGaiaInventoryManagerSettings* Settings = GetDefault<UGaiaInventoryManagerSettings>();
	if (!Settings)
	{
		UE_LOG(LogGaia, Error, TEXT("无法获取GaiaInventoryManagerSettings"));
		return false;
	}
	
	if (ContainerDefID == NAME_None)
	{
		UE_LOG(LogGaia, Warning, TEXT("ContainerDefID为空"));
		return false;
	}
	
	if (UDataRegistrySubsystem* DataRegistry = GEngine->GetEngineSubsystem<UDataRegistrySubsystem>())
	{
		const FGaiaContainerDefinition* ContainerDef = DataRegistry->GetCachedItem<FGaiaContainerDefinition>(
			FDataRegistryId(Settings->ContainerDefinitionRegistryType, ContainerDefID));
		
		if (ContainerDef)
		{
			OutContainerDef = *ContainerDef;
			return true;
		}
	}
	
	UE_LOG(LogGaia, Warning, TEXT("无法找到容器定义: %s"), *ContainerDefID.ToString());
	return false;
}

//~END 数据定义获取

//~BEGIN 实例创建

FGaiaItemInstance UGaiaInventorySubsystem::CreateItemInstance(FName ItemDefID, int32 Quantity)
{
	FGaiaItemInstance NewItem;
	
	// 获取物品定义
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(ItemDefID, ItemDef))
	{
		UE_LOG(LogGaia, Error, TEXT("无法创建物品实例，物品定义不存在: %s"), *ItemDefID.ToString());
		return NewItem;
	}
	
	// 生成唯一ID
	NewItem.InstanceUID = FGuid::NewGuid();
	NewItem.ItemDefinitionID = ItemDefID;
	
	// 设置数量（考虑堆叠规则）
	if (ItemDef.bStackable)
	{
		NewItem.Quantity = FMath::Clamp(Quantity, 1, ItemDef.MaxStackSize);
	}
	else
	{
		NewItem.Quantity = 1;
	}
	
	// 如果物品有容器，创建容器实例
	if (ItemDef.bHasContainer && ItemDef.ContainerDefinitionID != NAME_None)
	{
		FGuid ContainerUID = CreateContainer(ItemDef.ContainerDefinitionID);
		if (ContainerUID.IsValid())
		{
			NewItem.OwnedContainerUID = ContainerUID;
			
			// 设置容器的拥有者
			if (FGaiaContainerInstance* Container = Containers.Find(ContainerUID))
			{
				Container->OwnerItemUID = NewItem.InstanceUID;
			}
		}
	}
	
	UE_LOG(LogGaia, Log, TEXT("创建物品实例: %s, UID: %s, 数量: %d"), 
		*ItemDefID.ToString(), *NewItem.InstanceUID.ToString(), NewItem.Quantity);
	
	return NewItem;
}

FGuid UGaiaInventorySubsystem::CreateContainer(FName ContainerDefID)
{
	FGaiaContainerDefinition ContainerDef;
	if (!GetContainerDefinition(ContainerDefID, ContainerDef))
	{
		UE_LOG(LogGaia, Error, TEXT("无法创建容器实例，容器定义不存在: %s"), *ContainerDefID.ToString());
		return FGuid();
	}
	
	FGaiaContainerInstance NewContainer;
	
	// 生成唯一ID
	NewContainer.ContainerUID = FGuid::NewGuid();
	NewContainer.ContainerDefinitionID = ContainerDefID;
	
	// 初始化槽位
	NewContainer.Slots.Reserve(ContainerDef.SlotCount);
	for (int32 i = 0; i < ContainerDef.SlotCount; i++)
	{
		NewContainer.Slots.Add(FGaiaSlotInfo(i));
	}
	
	// 添加到容器映射表
	Containers.Add(NewContainer.ContainerUID, NewContainer);
	
	UE_LOG(LogGaia, Log, TEXT("创建容器实例: %s, UID: %s, 槽位数: %d"), 
		*ContainerDefID.ToString(), *NewContainer.ContainerUID.ToString(), ContainerDef.SlotCount);
	
	return NewContainer.ContainerUID;
}

//~END 实例创建

//~BEGIN 查询功能

bool UGaiaInventorySubsystem::FindItemByUID(const FGuid& InstanceUID, FGaiaItemInstance& OutItem)
{
	if (!InstanceUID.IsValid())
	{
		return false;
	}
	
	// 直接从全局物品池查找 O(1)
	const FGaiaItemInstance* Item = AllItems.Find(InstanceUID);
	if (!Item)
	{
		return false;
	}
	
	OutItem = *Item;
	return true;
}

bool UGaiaInventorySubsystem::FindContainerByUID(const FGuid& ContainerUID, FGaiaContainerInstance& OutContainer)
{
	if (!ContainerUID.IsValid())
	{
		return false;
	}
	
	FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (!Container)
	{
		return false;
	}
	
	OutContainer = *Container;
	return true;
}

//~END 查询功能

//~BEGIN 容器操作

bool UGaiaInventorySubsystem::AddItemToContainer(const FGaiaItemInstance& Item, const FGuid& ContainerUID)
{
	// 检查物品有效性
	if (!Item.IsValid())
	{
		UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 物品实例无效 (ItemDefID: %s, Quantity: %d)"), 
			*Item.ItemDefinitionID.ToString(), Item.Quantity);
		return false;
	}
	
	// 检查容器是否存在
	FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (!Container)
	{
		UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 容器不存在 (ContainerUID: %s)"), 
			*ContainerUID.ToString());
		return false;
	}
	
	// 获取容器定义
	FGaiaContainerDefinition ContainerDef;
	if (!GetContainerDefinition(Container->ContainerDefinitionID, ContainerDef))
	{
		UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 无法获取容器定义 (ContainerDefID: %s)"), 
			*Container->ContainerDefinitionID.ToString());
		return false;
	}
	
	// 检查槽位是否已满
	int32 UsedSlotCount = Container->GetUsedSlotCount();
	if (UsedSlotCount >= ContainerDef.SlotCount)
	{
		UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 容器槽位已满 (已用: %d/%d)"), 
			UsedSlotCount, ContainerDef.SlotCount);
		return false;
	}
	
	// 获取物品定义
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(Item.ItemDefinitionID, ItemDef))
	{
		UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 无法获取物品定义 (ItemDefID: %s)"), 
			*Item.ItemDefinitionID.ToString());
		return false;
	}
	
	// 检查物品标签是否允许
	if (!ContainerDef.AllowedItemTags.IsEmpty())
	{
		bool bHasMatchingTag = false;
		for (const FGameplayTag& ItemTag : ItemDef.ItemTags)
		{
			if (ContainerDef.HasAllowedTag(ItemTag))
			{
				bHasMatchingTag = true;
				break;
			}
		}
		
		if (!bHasMatchingTag)
		{
			UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 物品标签不匹配容器允许的标签 (ItemDefID: %s, 物品标签: %s, 允许标签: %s)"),
				*Item.ItemDefinitionID.ToString(),
				*ItemDef.ItemTags.ToStringSimple(),
				*ContainerDef.AllowedItemTags.ToStringSimple());
			return false;
		}
	}
	
	// 如果是容器物品，进行额外检查
	if (ItemDef.bHasContainer)
	{
		// 检查目标容器是否允许嵌套容器
		if (!ContainerDef.bAllowNestedContainers)
		{
			UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 目标容器不允许嵌套容器 (ItemDefID: %s, ContainerDefID: %s)"),
				*Item.ItemDefinitionID.ToString(),
				*Container->ContainerDefinitionID.ToString());
			return false;
		}
		
		// 检查物品体积是否可以放入容器
		if (ItemDef.ItemVolume >= ContainerDef.MaxVolume)
		{
			UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 物品体积太大，无法放入容器 (物品体积: %d, 容器内部体积: %d)"),
				ItemDef.ItemVolume, ContainerDef.MaxVolume);
			return false;
		}
		
		// 检查循环引用
		if (Item.HasContainer())
		{
			if (WouldCreateCycle(Item.OwnedContainerUID, ContainerUID))
			{
				UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 会造成容器循环引用 (物品容器UID: %s, 目标容器UID: %s)"),
					*Item.OwnedContainerUID.ToString(),
					*ContainerUID.ToString());
				return false;
			}
		}
	}
	
	// 检查体积限制
	if (ContainerDef.bEnableVolumeLimit)
	{
		int32 UsedVolume = GetContainerUsedVolume(ContainerUID);
		int32 ItemVolume = GetItemTotalVolume(Item);
		int32 AvailableVolume = ContainerDef.MaxVolume - UsedVolume;
		
		if (UsedVolume + ItemVolume > ContainerDef.MaxVolume)
		{
			UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 容器体积不足 (需要: %d, 剩余: %d, 总容量: %d)"),
				ItemVolume, AvailableVolume, ContainerDef.MaxVolume);
			return false;
		}
	}
	
	// TODO: 检查堆叠逻辑（如果物品可堆叠，尝试合并到现有堆）
	
	// 找到空槽位
	int32 EmptySlotID = Container->FindEmptySlotID();
	if (EmptySlotID == INDEX_NONE)
	{
		UE_LOG(LogGaia, Warning, TEXT("添加物品失败: 容器没有空槽位 (这不应该发生，可能是槽位状态不一致)"));
		return false;
	}
	
	// 复制物品实例，设置位置信息
	FGaiaItemInstance NewItem = Item;
	NewItem.CurrentContainerUID = ContainerUID;
	NewItem.CurrentSlotID = EmptySlotID;
	
	// 添加物品到全局池
	AllItems.Add(NewItem.InstanceUID, NewItem);
	
	// 更新槽位信息
	int32 SlotIndex = Container->GetSlotIndexByID(EmptySlotID);
	if (SlotIndex != INDEX_NONE)
	{
		Container->Slots[SlotIndex].ItemInstanceUID = NewItem.InstanceUID;
	}
	
	// 如果物品有容器，更新容器的父容器
	if (NewItem.HasContainer())
	{
		if (FGaiaContainerInstance* ItemContainer = Containers.Find(NewItem.OwnedContainerUID))
		{
			ItemContainer->ParentContainerUID = ContainerUID;
		}
	}
	
	// 标记需要重新计算
	Container->MarkDirty();
	
	UE_LOG(LogGaia, Log, TEXT("添加物品成功: 物品 '%s' (UID: %s, 数量: %d) -> 容器 '%s' (UID: %s, 槽位: %d/%d)"), 
		*ItemDef.ItemName.ToString(),
		*NewItem.InstanceUID.ToString(),
		NewItem.Quantity,
		*ContainerDef.ContainerName.ToString(),
		*ContainerUID.ToString(),
		UsedSlotCount + 1,
		ContainerDef.SlotCount);
	
	return true;
}

bool UGaiaInventorySubsystem::RemoveItemFromContainer(const FGuid& ItemUID)
{
	if (!ItemUID.IsValid())
	{
		return false;
	}
	
	// 从全局池查找物品
	FGaiaItemInstance* Item = AllItems.Find(ItemUID);
	if (!Item)
	{
		UE_LOG(LogGaia, Warning, TEXT("无法找到要移除的物品: %s"), *ItemUID.ToString());
		return false;
	}
	
	// 检查物品是否在容器中
	if (!Item->IsInContainer())
	{
		UE_LOG(LogGaia, Warning, TEXT("物品不在任何容器中: %s"), *ItemUID.ToString());
		return false;
	}
	
	// 获取物品所在容器
	FGaiaContainerInstance* Container = Containers.Find(Item->CurrentContainerUID);
	if (!Container)
	{
		UE_LOG(LogGaia, Error, TEXT("物品所在容器不存在: %s"), *Item->CurrentContainerUID.ToString());
		return false;
	}
	
	// 清空槽位引用
	int32 SlotIndex = Container->GetSlotIndexByID(Item->CurrentSlotID);
	if (SlotIndex != INDEX_NONE)
	{
		Container->Slots[SlotIndex].ItemInstanceUID = FGuid();
	}
	
	// 如果物品有容器，清空容器的父容器引用
	if (Item->HasContainer())
	{
		if (FGaiaContainerInstance* ItemContainer = Containers.Find(Item->OwnedContainerUID))
		{
			ItemContainer->ParentContainerUID = FGuid(); // 清空父容器引用
			UE_LOG(LogGaia, Log, TEXT("清空物品容器的父容器引用: %s"), *Item->OwnedContainerUID.ToString());
		}
	}
	
	// 将物品设为游离状态（不删除物品）
	FGuid OldContainerUID = Item->CurrentContainerUID;
	int32 OldSlotID = Item->CurrentSlotID;
	
	Item->CurrentContainerUID = FGuid(); // 无效 = 游离状态
	Item->CurrentSlotID = -1;
	
	// 标记需要重新计算
	Container->MarkDirty();
	
	UE_LOG(LogGaia, Log, TEXT("从容器移除物品: %s (容器: %s, 槽位: %d) -> 游离状态"), 
		*ItemUID.ToString(), *OldContainerUID.ToString(), OldSlotID);
	return true;
}

bool UGaiaInventorySubsystem::DestroyItem(const FGuid& ItemUID)
{
	if (!ItemUID.IsValid())
	{
		return false;
	}
	
	// 从全局池查找物品
	FGaiaItemInstance* Item = AllItems.Find(ItemUID);
	if (!Item)
	{
		UE_LOG(LogGaia, Warning, TEXT("无法找到要删除的物品: %s"), *ItemUID.ToString());
		return false;
	}
	
	// 如果物品在容器中，先从容器移除
	if (Item->IsInContainer())
	{
		RemoveItemFromContainer(ItemUID);
	}
	
	// 如果物品有容器，删除容器及其内容
	if (Item->HasContainer())
	{
		// 递归删除容器内的所有物品
		TArray<FGaiaItemInstance> ItemsInContainer = GetItemsInContainer(Item->OwnedContainerUID);
		for (const FGaiaItemInstance& ContainedItem : ItemsInContainer)
		{
			DestroyItem(ContainedItem.InstanceUID);
		}
		
		// 删除容器本身
		Containers.Remove(Item->OwnedContainerUID);
		UE_LOG(LogGaia, Log, TEXT("删除物品的容器: %s"), *Item->OwnedContainerUID.ToString());
	}
	
	// 从全局池删除物品
	AllItems.Remove(ItemUID);
	
	UE_LOG(LogGaia, Log, TEXT("完全删除物品: %s"), *ItemUID.ToString());
	return true;
}

bool UGaiaInventorySubsystem::CanAddItemToContainer(const FGaiaItemInstance& Item, const FGuid& ContainerUID) const
{
	// 检查容器是否存在
	const FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (!Container)
	{
		return false;
	}
	
	// 获取容器定义
	FGaiaContainerDefinition ContainerDef;
	if (!GetContainerDefinition(Container->ContainerDefinitionID, ContainerDef))
	{
		return false;
	}
	
	// 检查是否有空槽位
	if (Container->GetUsedSlotCount() >= ContainerDef.SlotCount)
	{
		return false;
	}
	
	// 获取物品定义
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(Item.ItemDefinitionID, ItemDef))
	{
		return false;
	}
	
	// 检查物品标签是否允许
	if (!ContainerDef.AllowedItemTags.IsEmpty())
	{
		bool bHasMatchingTag = false;
		for (const FGameplayTag& ItemTag : ItemDef.ItemTags)
		{
			if (ContainerDef.HasAllowedTag(ItemTag))
			{
				bHasMatchingTag = true;
				break;
			}
		}
		
		if (!bHasMatchingTag)
		{
			return false;  // 物品标签不匹配
		}
	}
	
	// 如果是容器物品，进行额外检查
	if (ItemDef.bHasContainer)
	{
		// 检查目标容器是否允许嵌套容器
		if (!ContainerDef.bAllowNestedContainers)
		{
			return false;
		}
		
		// 检查物品体积是否可以放入容器
		// 物品体积必须小于容器内部体积
		if (ItemDef.ItemVolume >= ContainerDef.MaxVolume)
		{
			return false;  // 物品太大，放不进去
		}
		
		// 检查循环引用
		if (Item.HasContainer())
		{
			if (WouldCreateCycle(Item.OwnedContainerUID, ContainerUID))
			{
				return false;  // 会造成循环引用
			}
		}
	}
	
	// 检查体积限制
	if (ContainerDef.bEnableVolumeLimit)
	{
		int32 UsedVolume = GetContainerUsedVolume(ContainerUID);
		int32 ItemVolume = GetItemTotalVolume(Item);
		
		if (UsedVolume + ItemVolume > ContainerDef.MaxVolume)
		{
			return false;  // 体积不足
		}
	}
	
	return true;
}

//~END 容器操作

//~BEGIN 嵌套检测

bool UGaiaInventorySubsystem::WouldCreateCycle(const FGuid& ItemContainerUID, const FGuid& TargetContainerUID) const
{
	if (!ItemContainerUID.IsValid() || !TargetContainerUID.IsValid())
	{
		return false;
	}
	
	// 向上遍历目标容器的所有父容器
	FGuid CurrentUID = TargetContainerUID;
	TSet<FGuid> Visited;  // 防止无限循环
	
	while (CurrentUID.IsValid())
	{
		// 检查是否已经访问过（检测循环）
		if (Visited.Contains(CurrentUID))
		{
			UE_LOG(LogGaia, Warning, TEXT("检测到容器循环引用"));
			return true;
		}
		
		// 检查是否等于物品的容器（会造成循环）
		if (CurrentUID == ItemContainerUID)
		{
			return true;
		}
		
		Visited.Add(CurrentUID);
		
		// 获取父容器
		const FGaiaContainerInstance* Container = Containers.Find(CurrentUID);
		if (!Container)
		{
			break;
		}
		
		// 如果容器属于某个物品，继续向上查找
		if (Container->OwnerItemUID.IsValid())
		{
			// 使用新架构：从AllItems查找拥有此容器的物品
			const FGaiaItemInstance* OwnerItem = AllItems.Find(Container->OwnerItemUID);
			
			if (OwnerItem && OwnerItem->IsInContainer())
			{
				// 物品在容器中，继续向上遍历
				CurrentUID = OwnerItem->CurrentContainerUID;
			}
			else
			{
				// 物品不在容器中（游离状态）或不存在，停止遍历
				break;
			}
		}
		else
		{
			// 容器不属于任何物品（顶层容器），没有父容器了
			break;
		}
	}
	
	return false;  // 没有循环
}

//~END 嵌套检测

//~BEGIN 体积/重量计算

int32 UGaiaInventorySubsystem::GetItemTotalVolume(const FGaiaItemInstance& Item) const
{
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(Item.ItemDefinitionID, ItemDef))
	{
		return 0;
	}
	
	int32 BaseVolume = ItemDef.ItemVolume * Item.Quantity;
	
	// 如果物品有容器，递归计算内容物体积
	if (Item.HasContainer())
	{
		int32 ContainerVolume = GetContainerUsedVolume(Item.OwnedContainerUID);
		
		// 物品占用的体积 = Max(物品自身体积, 物品体积 + 容器内容物体积)
		return FMath::Max(BaseVolume, ItemDef.ItemVolume + ContainerVolume);
	}
	
	return BaseVolume;
}

int32 UGaiaInventorySubsystem::GetItemTotalWeight(const FGaiaItemInstance& Item) const
{
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(Item.ItemDefinitionID, ItemDef))
	{
		return 0;
	}
	
	int32 BaseWeight = ItemDef.ItemWeight * Item.Quantity;
	
	// 如果物品有容器，递归计算内容物重量
	if (Item.HasContainer())
	{
		int32 ContainerWeight = GetContainerUsedWeight(Item.OwnedContainerUID);
		BaseWeight += ContainerWeight;
	}
	
	return BaseWeight;
}

int32 UGaiaInventorySubsystem::GetContainerUsedVolume(const FGuid& ContainerUID) const
{
	const FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (!Container)
	{
		return 0;
	}
	
	// 使用缓存（如果有效）
	if (!Container->bNeedRecalculate && Container->CachedTotalVolume > 0)
	{
		return Container->CachedTotalVolume;
	}
	
	// 遍历容器中的所有槽位，计算物品总体积
	int32 TotalVolume = 0;
	for (const FGaiaSlotInfo& Slot : Container->Slots)
	{
		if (!Slot.IsEmpty())
		{
			if (const FGaiaItemInstance* Item = AllItems.Find(Slot.ItemInstanceUID))
			{
				TotalVolume += GetItemTotalVolume(*Item);
			}
		}
	}
	
	// 更新缓存（注意：这里是const函数，实际实现可能需要mutable）
	// Container->CachedTotalVolume = TotalVolume;
	// Container->bNeedRecalculate = false;
	
	return TotalVolume;
}

int32 UGaiaInventorySubsystem::GetContainerUsedWeight(const FGuid& ContainerUID) const
{
	const FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (!Container)
	{
		return 0;
	}
	
	// 使用缓存（如果有效）
	if (!Container->bNeedRecalculate && Container->CachedTotalWeight > 0)
	{
		return Container->CachedTotalWeight;
	}
	
	// 遍历容器中的所有槽位，计算物品总重量
	int32 TotalWeight = 0;
	for (const FGaiaSlotInfo& Slot : Container->Slots)
	{
		if (!Slot.IsEmpty())
		{
			if (const FGaiaItemInstance* Item = AllItems.Find(Slot.ItemInstanceUID))
			{
				TotalWeight += GetItemTotalWeight(*Item);
			}
		}
	}
	
	// 更新缓存
	// Container->CachedTotalWeight = TotalWeight;
	// Container->bNeedRecalculate = false;
	
	return TotalWeight;
}

//~END 体积/重量计算

//~BEGIN 物品移动

FMoveItemResult UGaiaInventorySubsystem::MoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID, int32 Quantity)
{
	FMoveItemResult Result;
	
	UE_LOG(LogGaia, Log, TEXT("开始移动物品: ItemUID=%s, TargetContainer=%s, TargetSlot=%d, Quantity=%d"), 
		*ItemUID.ToString(), *TargetContainerUID.ToString(), TargetSlotID, Quantity);
	
	// 检查源物品是否存在
	FGaiaItemInstance SourceItem;
	if (!FindItemByUID(ItemUID, SourceItem))
	{
		Result.ErrorMessage = FString::Printf(TEXT("无法找到源物品: %s"), *ItemUID.ToString());
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Warning, TEXT("移动物品失败: 源物品不存在 - %s"), *Result.ErrorMessage);
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("找到源物品: %s (数量: %d)"), 
		*SourceItem.ItemDefinitionID.ToString(), SourceItem.Quantity);
	
	// 检查目标容器是否存在
	FGaiaContainerInstance* TargetContainer = Containers.Find(TargetContainerUID);
	if (!TargetContainer)
	{
		Result.ErrorMessage = FString::Printf(TEXT("无法找到目标容器: %s"), *TargetContainerUID.ToString());
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Warning, TEXT("移动物品失败: 目标容器不存在 - %s"), *Result.ErrorMessage);
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("找到目标容器: %s (槽位数: %d)"), 
		*TargetContainer->ContainerDefinitionID.ToString(), TargetContainer->Slots.Num());
	
	// 设置默认数量（移动全部）
	if (Quantity <= 0)
	{
		Quantity = SourceItem.Quantity;
		UE_LOG(LogGaia, Log, TEXT("设置移动数量为全部: %d"), Quantity);
	}
	
	// 检查数量是否有效
	if (Quantity > SourceItem.Quantity)
	{
		Result.ErrorMessage = FString::Printf(TEXT("移动数量 (%d) 超过源物品数量 (%d)"), Quantity, SourceItem.Quantity);
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Warning, TEXT("移动物品失败: 数量无效 - %s"), *Result.ErrorMessage);
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("移动数量验证通过: %d"), Quantity);
	
	// 检查源物品是否在容器中
	if (!SourceItem.IsInContainer())
	{
		Result.ErrorMessage = FString::Printf(TEXT("源物品不在任何容器中: %s"), *ItemUID.ToString());
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("移动物品失败: 源物品处于游离状态 - %s"), *Result.ErrorMessage);
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("找到源物品位置: Container=%s, SlotID=%d"), 
		*SourceItem.CurrentContainerUID.ToString(), SourceItem.CurrentSlotID);
	
	FGaiaContainerInstance* SourceContainer = Containers.Find(SourceItem.CurrentContainerUID);
	if (!SourceContainer)
	{
		Result.ErrorMessage = FString::Printf(TEXT("无法找到源容器: %s"), *SourceItem.CurrentContainerUID.ToString());
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("移动物品失败: 源容器不存在 - %s"), *Result.ErrorMessage);
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("找到源容器: %s"), *SourceContainer->ContainerDefinitionID.ToString());
	
	// 如果是同一个容器内的移动，需要特殊处理
	if (SourceItem.CurrentContainerUID == TargetContainerUID)
	{
		UE_LOG(LogGaia, Log, TEXT("执行容器内移动: 源容器 = 目标容器"));
		return MoveItemWithinContainer(ItemUID, TargetSlotID, Quantity);
	}
	
	UE_LOG(LogGaia, Log, TEXT("执行跨容器移动: %s -> %s"), 
		*SourceItem.CurrentContainerUID.ToString(), *TargetContainerUID.ToString());
	
	// 检查目标槽位
	if (TargetSlotID >= 0)
	{
		UE_LOG(LogGaia, Log, TEXT("指定目标槽位: %d"), TargetSlotID);
		
		// 指定槽位移动
		int32 TargetSlotIndex = TargetContainer->GetSlotIndexByID(TargetSlotID);
		if (TargetSlotIndex == INDEX_NONE)
		{
			Result.ErrorMessage = FString::Printf(TEXT("目标槽位无效: %d"), TargetSlotID);
			Result.Result = EMoveItemResult::InvalidTarget;
			UE_LOG(LogGaia, Warning, TEXT("移动物品失败: 目标槽位无效 - %s"), *Result.ErrorMessage);
			return Result;
		}
		
		FGaiaSlotInfo& TargetSlot = TargetContainer->Slots[TargetSlotIndex];
		
		if (TargetSlot.IsEmpty())
		{
			UE_LOG(LogGaia, Log, TEXT("目标槽位为空，执行直接移动"));
			// 目标槽位为空，直接移动
			return MoveToEmptySlot(ItemUID, TargetContainerUID, TargetSlotID, Quantity);
		}
		else
		{
			UE_LOG(LogGaia, Log, TEXT("目标槽位有物品: %s"), *TargetSlot.ItemInstanceUID.ToString());
			// 目标槽位有物品，需要处理
			FGaiaItemInstance TargetItem;
			if (FindItemByUID(TargetSlot.ItemInstanceUID, TargetItem))
			{
				UE_LOG(LogGaia, Log, TEXT("找到目标物品: %s (数量: %d)"), 
					*TargetItem.ItemDefinitionID.ToString(), TargetItem.Quantity);
				return ProcessTargetSlotWithItem(ItemUID, TargetItem, TargetContainerUID, TargetSlotID, Quantity);
			}
			else
			{
				Result.ErrorMessage = FString::Printf(TEXT("无法找到目标槽位中的物品: %s"), *TargetSlot.ItemInstanceUID.ToString());
				Result.Result = EMoveItemResult::Failed;
				UE_LOG(LogGaia, Error, TEXT("移动物品失败: 目标槽位物品丢失 - %s"), *Result.ErrorMessage);
				return Result;
			}
		}
	}
	else
	{
		UE_LOG(LogGaia, Log, TEXT("自动分配槽位移动"));
		// 自动分配槽位
		return MoveItemAutoSlot(ItemUID, TargetContainerUID, Quantity);
	}
}

bool UGaiaInventorySubsystem::CanMoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID, int32 Quantity)
{
	// 检查源物品是否存在
	FGaiaItemInstance SourceItem;
	if (!FindItemByUID(ItemUID, SourceItem))
	{
		return false;
	}
	
	// 检查目标容器是否存在
	const FGaiaContainerInstance* TargetContainer = Containers.Find(TargetContainerUID);
	if (!TargetContainer)
	{
		return false;
	}
	
	// 设置默认数量
	if (Quantity <= 0)
	{
		Quantity = SourceItem.Quantity;
	}
	
	// 检查数量是否有效
	if (Quantity > SourceItem.Quantity)
	{
		return false;
	}
	
	// 检查是否可以添加到目标容器
	FGaiaItemInstance TestItem = SourceItem;
	TestItem.Quantity = Quantity;
	
	return CanAddItemToContainer(TestItem, TargetContainerUID);
}

FMoveItemResult UGaiaInventorySubsystem::QuickMoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID)
{
	return MoveItem(ItemUID, TargetContainerUID, TargetSlotID, -1); // -1 表示移动全部
}

FMoveItemResult UGaiaInventorySubsystem::SplitItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 Quantity, int32 TargetSlotID)
{
	if (Quantity <= 0)
	{
		FMoveItemResult Result;
		Result.ErrorMessage = TEXT("拆分数量必须大于0");
		Result.Result = EMoveItemResult::Failed;
		return Result;
	}
	
	return MoveItem(ItemUID, TargetContainerUID, TargetSlotID, Quantity);
}

//~END 物品移动

//~BEGIN 移动辅助函数

FMoveItemResult UGaiaInventorySubsystem::TryStackItems(const FGuid& SourceItemUID, const FGuid& TargetItemUID, int32 Quantity)
{
	FMoveItemResult Result;
	
	UE_LOG(LogGaia, Log, TEXT("尝试堆叠物品: Source=%s, Target=%s, Quantity=%d"), 
		*SourceItemUID.ToString(), *TargetItemUID.ToString(), Quantity);
	
	// 获取源物品和目标物品
	FGaiaItemInstance SourceItem, TargetItem;
	if (!FindItemByUID(SourceItemUID, SourceItem) || !FindItemByUID(TargetItemUID, TargetItem))
	{
		Result.ErrorMessage = TEXT("无法找到源物品或目标物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Warning, TEXT("堆叠失败: 无法找到源物品或目标物品"));
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("源物品: %s (数量: %d), 目标物品: %s (数量: %d)"), 
		*SourceItem.ItemDefinitionID.ToString(), SourceItem.Quantity,
		*TargetItem.ItemDefinitionID.ToString(), TargetItem.Quantity);
	
	// 检查是否为相同类型
	if (SourceItem.ItemDefinitionID != TargetItem.ItemDefinitionID)
	{
		Result.ErrorMessage = TEXT("物品类型不匹配，无法堆叠");
		Result.Result = EMoveItemResult::TypeMismatch;
		UE_LOG(LogGaia, Warning, TEXT("堆叠失败: 物品类型不匹配 - 源: %s, 目标: %s"), 
			*SourceItem.ItemDefinitionID.ToString(), *TargetItem.ItemDefinitionID.ToString());
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("物品类型匹配，继续检查堆叠规则"));
	
	// 获取物品定义
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(SourceItem.ItemDefinitionID, ItemDef))
	{
		Result.ErrorMessage = TEXT("无法获取物品定义");
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("堆叠失败: 无法获取物品定义 - %s"), *SourceItem.ItemDefinitionID.ToString());
		return Result;
	}
	
	// 检查是否可堆叠
	if (!ItemDef.bStackable)
	{
		Result.ErrorMessage = TEXT("物品不可堆叠");
		Result.Result = EMoveItemResult::StackLimitReached;
		UE_LOG(LogGaia, Warning, TEXT("堆叠失败: 物品不可堆叠 - %s"), *SourceItem.ItemDefinitionID.ToString());
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("物品可堆叠，最大堆叠: %d"), ItemDef.MaxStackSize);
	
	// 计算可堆叠的数量
	int32 AvailableSpace = ItemDef.MaxStackSize - TargetItem.Quantity;
	int32 StackQuantity = FMath::Min(Quantity, AvailableSpace);
	
	UE_LOG(LogGaia, Log, TEXT("堆叠计算: 可用空间=%d, 请求数量=%d, 实际堆叠=%d"), 
		AvailableSpace, Quantity, StackQuantity);
	
	if (StackQuantity <= 0)
	{
		Result.ErrorMessage = TEXT("目标物品堆叠已满");
		Result.Result = EMoveItemResult::StackLimitReached;
		UE_LOG(LogGaia, Warning, TEXT("堆叠失败: 目标物品堆叠已满 - 当前数量: %d, 最大堆叠: %d"), 
			TargetItem.Quantity, ItemDef.MaxStackSize);
		return Result;
	}
	
	// 使用新架构：直接从AllItems获取目标物品
	FGaiaItemInstance* TargetItemPtr = AllItems.Find(TargetItemUID);
	if (!TargetItemPtr)
	{
		Result.ErrorMessage = TEXT("无法找到目标物品");
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("堆叠失败: 无法找到目标物品 - %s"), *TargetItemUID.ToString());
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("开始执行堆叠: 目标物品=%s, 当前数量=%d"), 
		*TargetItemPtr->ItemDefinitionID.ToString(), TargetItemPtr->Quantity);
	
	// 更新目标物品数量
	int32 OldTargetQuantity = TargetItemPtr->Quantity;
	TargetItemPtr->Quantity += StackQuantity;
	
	UE_LOG(LogGaia, Log, TEXT("更新目标物品数量: %d -> %d"), 
		OldTargetQuantity, TargetItemPtr->Quantity);
	
	// 更新源物品数量
	if (FGaiaItemInstance* SourceItemPtr = AllItems.Find(SourceItemUID))
	{
		int32 OldSourceQuantity = SourceItemPtr->Quantity;
		SourceItemPtr->Quantity -= StackQuantity;
		
		UE_LOG(LogGaia, Log, TEXT("更新源物品数量: %d -> %d"), 
			OldSourceQuantity, SourceItemPtr->Quantity);
		
		// 如果源物品数量为0，删除源物品
		if (SourceItemPtr->Quantity <= 0)
		{
			UE_LOG(LogGaia, Log, TEXT("源物品数量为0，删除源物品"));
			DestroyItem(SourceItemUID);
		}
		else
		{
			// 标记源容器需要重新计算
			if (SourceItemPtr->IsInContainer())
			{
				if (FGaiaContainerInstance* SourceContainer = Containers.Find(SourceItemPtr->CurrentContainerUID))
				{
					SourceContainer->MarkDirty();
				}
			}
		}
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("堆叠失败: 无法找到源物品 - %s"), *SourceItemUID.ToString());
	}
	
	// 标记目标容器需要重新计算
	if (TargetItemPtr->IsInContainer())
	{
		if (FGaiaContainerInstance* TargetContainer = Containers.Find(TargetItemPtr->CurrentContainerUID))
		{
			TargetContainer->MarkDirty();
		}
	}
	
	// 设置结果
	Result.Result = (StackQuantity == Quantity) ? EMoveItemResult::Success : EMoveItemResult::PartialSuccess;
	Result.MovedQuantity = StackQuantity;
	Result.RemainingQuantity = Quantity - StackQuantity;
	Result.TargetContainerUID = TargetItemPtr->CurrentContainerUID;
	
	if (Result.Result == EMoveItemResult::Success)
	{
		UE_LOG(LogGaia, Log, TEXT("堆叠成功: 完全移动了 %d 个物品到目标堆叠"), StackQuantity);
	}
	else
	{
		UE_LOG(LogGaia, Log, TEXT("部分堆叠成功: 移动了 %d 个物品，剩余 %d 个"), 
			StackQuantity, Result.RemainingQuantity);
	}
	
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::TryMoveToContainer(const FGuid& ItemUID, const FGuid& ContainerItemUID, int32 Quantity)
{
	FMoveItemResult Result;
	
	UE_LOG(LogGaia, Log, TEXT("尝试将物品放入容器: Item=%s, ContainerItem=%s, Quantity=%d"), 
		*ItemUID.ToString(), *ContainerItemUID.ToString(), Quantity);
	
	// 获取容器物品
	FGaiaItemInstance ContainerItem;
	if (!FindItemByUID(ContainerItemUID, ContainerItem))
	{
		Result.ErrorMessage = TEXT("无法找到容器物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Warning, TEXT("放入容器失败: 无法找到容器物品 - %s"), *ContainerItemUID.ToString());
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("找到容器物品: %s"), *ContainerItem.ItemDefinitionID.ToString());
	
	// 检查容器物品是否有容器
	if (!ContainerItem.HasContainer())
	{
		Result.ErrorMessage = TEXT("目标物品没有容器功能");
		Result.Result = EMoveItemResult::ContainerRejected;
		UE_LOG(LogGaia, Warning, TEXT("放入容器失败: 目标物品没有容器功能 - %s"), *ContainerItem.ItemDefinitionID.ToString());
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("容器物品有容器功能，容器UID: %s"), *ContainerItem.OwnedContainerUID.ToString());
	
	// 获取要移动的物品
	FGaiaItemInstance ItemToMove;
	if (!FindItemByUID(ItemUID, ItemToMove))
	{
		Result.ErrorMessage = TEXT("无法找到要移动的物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Warning, TEXT("放入容器失败: 无法找到要移动的物品 - %s"), *ItemUID.ToString());
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("找到要移动的物品: %s (数量: %d)"), 
		*ItemToMove.ItemDefinitionID.ToString(), ItemToMove.Quantity);
	
	// 设置移动数量
	if (Quantity <= 0)
	{
		Quantity = ItemToMove.Quantity;
		UE_LOG(LogGaia, Log, TEXT("设置移动数量为全部: %d"), Quantity);
	}
	
	// 创建测试物品
	FGaiaItemInstance TestItem = ItemToMove;
	TestItem.Quantity = Quantity;
	
	UE_LOG(LogGaia, Log, TEXT("检查是否可以添加到容器: %s"), *ContainerItem.OwnedContainerUID.ToString());
	
	// 检查是否可以添加到容器
	if (!CanAddItemToContainer(TestItem, ContainerItem.OwnedContainerUID))
	{
		Result.ErrorMessage = TEXT("无法将物品放入目标容器");
		Result.Result = EMoveItemResult::ContainerRejected;
		UE_LOG(LogGaia, Warning, TEXT("放入容器失败: 无法将物品放入目标容器 - 物品: %s, 容器: %s"), 
			*ItemToMove.ItemDefinitionID.ToString(), *ContainerItem.OwnedContainerUID.ToString());
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("容器检查通过，开始执行移动"));
	
	// 执行移动
	UE_LOG(LogGaia, Log, TEXT("调用AddItemToContainer添加物品到容器"));
	if (AddItemToContainer(TestItem, ContainerItem.OwnedContainerUID))
	{
		UE_LOG(LogGaia, Log, TEXT("AddItemToContainer成功，开始更新源物品数量"));
		
		// 使用新架构：直接从AllItems获取源物品
		if (FGaiaItemInstance* SourceItemPtr = AllItems.Find(ItemUID))
		{
			int32 OldQuantity = SourceItemPtr->Quantity;
			SourceItemPtr->Quantity -= Quantity;
			
			UE_LOG(LogGaia, Log, TEXT("更新源物品数量: %d -> %d"), 
				OldQuantity, SourceItemPtr->Quantity);
			
			// 如果源物品数量为0，删除源物品
			if (SourceItemPtr->Quantity <= 0)
			{
				UE_LOG(LogGaia, Log, TEXT("源物品数量为0，删除源物品"));
				DestroyItem(ItemUID);
			}
			else
			{
				// 标记源容器需要重新计算
				if (SourceItemPtr->IsInContainer())
				{
					if (FGaiaContainerInstance* SourceContainer = Containers.Find(SourceItemPtr->CurrentContainerUID))
					{
						SourceContainer->MarkDirty();
					}
				}
			}
		}
		else
		{
			UE_LOG(LogGaia, Error, TEXT("放入容器失败: 无法找到源物品 - %s"), *ItemUID.ToString());
		}
		
		Result.Result = EMoveItemResult::Success;
		Result.MovedQuantity = Quantity;
		Result.RemainingQuantity = 0;
		Result.bMovedToContainer = true;
		Result.TargetContainerUID = ContainerItem.OwnedContainerUID;
		
		UE_LOG(LogGaia, Log, TEXT("成功将物品放入容器: %s (数量: %d)"), *ContainerItemUID.ToString(), Quantity);
	}
	else
	{
		Result.ErrorMessage = TEXT("添加物品到容器失败");
		Result.Result = EMoveItemResult::ContainerRejected;
		UE_LOG(LogGaia, Warning, TEXT("放入容器失败: AddItemToContainer返回false - 物品: %s, 容器: %s"), 
			*ItemToMove.ItemDefinitionID.ToString(), *ContainerItem.OwnedContainerUID.ToString());
	}
	
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::SwapItems(const FGuid& ItemUID1, const FGuid& ItemUID2)
{
	FMoveItemResult Result;
	
	UE_LOG(LogGaia, Log, TEXT("尝试交换物品: Item1=%s, Item2=%s"), 
		*ItemUID1.ToString(), *ItemUID2.ToString());
	
	// 获取两个物品
	FGaiaItemInstance Item1, Item2;
	if (!FindItemByUID(ItemUID1, Item1) || !FindItemByUID(ItemUID2, Item2))
	{
		Result.ErrorMessage = TEXT("无法找到要交换的物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Warning, TEXT("交换失败: 无法找到要交换的物品 - Item1: %s, Item2: %s"), 
			*ItemUID1.ToString(), *ItemUID2.ToString());
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("找到要交换的物品: Item1=%s (数量: %d), Item2=%s (数量: %d)"), 
		*Item1.ItemDefinitionID.ToString(), Item1.Quantity,
		*Item2.ItemDefinitionID.ToString(), Item2.Quantity);
	
	// 检查是否可以交换
	if (!CanSwapItems(ItemUID1, ItemUID2))
	{
		Result.ErrorMessage = TEXT("无法交换物品");
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Warning, TEXT("交换失败: 无法交换物品 - 可能的原因: 容器限制、体积限制等"));
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("交换检查通过，开始执行交换"));
	
	// 使用新架构：直接从AllItems获取物品并交换它们的位置信息
	FGaiaItemInstance* ItemPtr1 = AllItems.Find(ItemUID1);
	FGaiaItemInstance* ItemPtr2 = AllItems.Find(ItemUID2);
	
	if (!ItemPtr1 || !ItemPtr2)
	{
		Result.ErrorMessage = TEXT("无法找到物品");
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("交换失败: 无法找到物品实例"));
		return Result;
	}
	
	// 获取原始位置
	FGuid Container1UID = ItemPtr1->CurrentContainerUID;
	int32 Slot1ID = ItemPtr1->CurrentSlotID;
	FGuid Container2UID = ItemPtr2->CurrentContainerUID;
	int32 Slot2ID = ItemPtr2->CurrentSlotID;
	
	UE_LOG(LogGaia, Log, TEXT("找到物品位置: Item1在容器%s槽位%d, Item2在容器%s槽位%d"), 
		*Container1UID.ToString(), Slot1ID,
		*Container2UID.ToString(), Slot2ID);
	
	// 获取容器
	FGaiaContainerInstance* Container1 = Containers.Find(Container1UID);
	FGaiaContainerInstance* Container2 = Containers.Find(Container2UID);
	
	if (!Container1 || !Container2)
	{
		Result.ErrorMessage = TEXT("无法找到容器");
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("交换失败: 无法找到容器"));
		return Result;
	}
	
	// 执行交换：交换物品的位置信息
	UE_LOG(LogGaia, Log, TEXT("开始执行物品交换"));
	
	// 交换物品的位置
	ItemPtr1->CurrentContainerUID = Container2UID;
	ItemPtr1->CurrentSlotID = Slot2ID;
	ItemPtr2->CurrentContainerUID = Container1UID;
	ItemPtr2->CurrentSlotID = Slot1ID;
	
	// 交换槽位引用
	int32 SlotIndex1 = Container1->GetSlotIndexByID(Slot1ID);
	int32 SlotIndex2 = Container2->GetSlotIndexByID(Slot2ID);
	
	if (SlotIndex1 != INDEX_NONE)
	{
		Container1->Slots[SlotIndex1].ItemInstanceUID = ItemUID2;
		UE_LOG(LogGaia, Log, TEXT("Container1槽位%d更新: %s -> %s"), Slot1ID, *ItemUID1.ToString(), *ItemUID2.ToString());
	}
	
	if (SlotIndex2 != INDEX_NONE)
	{
		Container2->Slots[SlotIndex2].ItemInstanceUID = ItemUID1;
		UE_LOG(LogGaia, Log, TEXT("Container2槽位%d更新: %s -> %s"), Slot2ID, *ItemUID2.ToString(), *ItemUID1.ToString());
	}
	
	// 标记需要重新计算
	UE_LOG(LogGaia, Log, TEXT("标记容器需要重新计算"));
	Container1->MarkDirty();
	Container2->MarkDirty();
	
	Result.Result = EMoveItemResult::SwapPerformed;
	Result.MovedQuantity = Item1.Quantity;
	Result.RemainingQuantity = 0;
	Result.bWasSwapped = true;
	Result.SwappedItemUID = ItemUID2;
	
	UE_LOG(LogGaia, Log, TEXT("交换物品成功: %s <-> %s"), *ItemUID1.ToString(), *ItemUID2.ToString());
	
	return Result;
}

bool UGaiaInventorySubsystem::CanSwapItems(const FGuid& ItemUID1, const FGuid& ItemUID2)
{
	// 使用新架构：直接从AllItems获取物品
	const FGaiaItemInstance* Item1 = AllItems.Find(ItemUID1);
	const FGaiaItemInstance* Item2 = AllItems.Find(ItemUID2);
	
	if (!Item1 || !Item2)
	{
		return false;
	}
	
	// 检查是否都在容器中
	if (!Item1->IsInContainer() || !Item2->IsInContainer())
	{
		return false;
	}
	
	// 检查是否可以添加到对方的容器
	return CanAddItemToContainer(*Item1, Item2->CurrentContainerUID) && 
		   CanAddItemToContainer(*Item2, Item1->CurrentContainerUID);
}

FMoveItemResult UGaiaInventorySubsystem::MoveToEmptySlot(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID, int32 Quantity)
{
	FMoveItemResult Result;
	
	// 获取源物品
	FGaiaItemInstance SourceItem;
	if (!FindItemByUID(ItemUID, SourceItem))
	{
		Result.ErrorMessage = TEXT("无法找到源物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	// 设置移动数量
	if (Quantity <= 0)
	{
		Quantity = SourceItem.Quantity;
	}
	
	// 如果是部分移动，需要创建新的物品实例
	bool bIsPartialMove = (Quantity < SourceItem.Quantity);
	FGaiaItemInstance ItemToMove = SourceItem;
	ItemToMove.Quantity = Quantity;
	
	// 如果是部分移动，生成新的UID
	if (bIsPartialMove)
	{
		ItemToMove.InstanceUID = FGuid::NewGuid();
		UE_LOG(LogGaia, Log, TEXT("部分移动，创建新物品实例: 原UID=%s, 新UID=%s, 数量=%d"), 
			*SourceItem.InstanceUID.ToString(), *ItemToMove.InstanceUID.ToString(), Quantity);
	}
	
	// 检查是否可以添加到目标容器
	if (!CanAddItemToContainer(ItemToMove, TargetContainerUID))
	{
		Result.ErrorMessage = TEXT("无法将物品添加到目标容器");
		Result.Result = EMoveItemResult::Failed;
		return Result;
	}
	
	// 添加到目标容器
	if (AddItemToContainer(ItemToMove, TargetContainerUID))
	{
		// 使用新架构：更新源物品数量
		if (FGaiaItemInstance* SourceItemPtr = AllItems.Find(ItemUID))
		{
			SourceItemPtr->Quantity -= Quantity;
			
			// 如果源物品数量为0，删除源物品
			if (SourceItemPtr->Quantity <= 0)
			{
				DestroyItem(ItemUID);
			}
			else
			{
				// 标记源容器需要重新计算
				if (SourceItemPtr->IsInContainer())
				{
					if (FGaiaContainerInstance* SourceContainer = Containers.Find(SourceItemPtr->CurrentContainerUID))
					{
						SourceContainer->MarkDirty();
					}
				}
			}
		}
		
		Result.Result = EMoveItemResult::Success;
		Result.MovedQuantity = Quantity;
		Result.RemainingQuantity = 0;
		Result.TargetContainerUID = TargetContainerUID;
		
		UE_LOG(LogGaia, Log, TEXT("成功移动物品到空槽位: %s"), *ItemUID.ToString());
	}
	else
	{
		Result.ErrorMessage = TEXT("添加物品到容器失败");
		Result.Result = EMoveItemResult::Failed;
	}
	
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::MoveItemWithinContainer(const FGuid& ItemUID, int32 TargetSlotID, int32 Quantity)
{
	FMoveItemResult Result;
	
	// 使用新架构：从AllItems获取源物品
	FGaiaItemInstance* SourceItemPtr = AllItems.Find(ItemUID);
	if (!SourceItemPtr)
	{
		Result.ErrorMessage = TEXT("无法找到源物品");
		Result.Result = EMoveItemResult::Failed;
		return Result;
	}
	
	// 检查物品是否在容器中
	if (!SourceItemPtr->IsInContainer())
	{
		Result.ErrorMessage = TEXT("源物品不在容器中");
		Result.Result = EMoveItemResult::Failed;
		return Result;
	}
	
	FGaiaContainerInstance* Container = Containers.Find(SourceItemPtr->CurrentContainerUID);
	if (!Container)
	{
		Result.ErrorMessage = TEXT("无法找到容器");
		Result.Result = EMoveItemResult::Failed;
		return Result;
	}
	
	// 检查目标槽位
	if (TargetSlotID < 0)
	{
		Result.ErrorMessage = TEXT("容器内移动必须指定目标槽位");
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	int32 TargetSlotIndex = Container->GetSlotIndexByID(TargetSlotID);
	if (TargetSlotIndex == INDEX_NONE)
	{
		Result.ErrorMessage = FString::Printf(TEXT("目标槽位无效: %d"), TargetSlotID);
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	FGaiaSlotInfo& TargetSlot = Container->Slots[TargetSlotIndex];
	
	if (TargetSlot.IsEmpty())
	{
		// 目标槽位为空，直接移动
		// 设置移动数量
		if (Quantity <= 0 || Quantity > SourceItemPtr->Quantity)
		{
			Quantity = SourceItemPtr->Quantity;
		}

		if (bool bIsPartialMove = (Quantity < SourceItemPtr->Quantity))
		{
			// 部分移动：创建新物品实例
			FGaiaItemInstance ItemToMove = *SourceItemPtr;
			ItemToMove.InstanceUID = FGuid::NewGuid();
			ItemToMove.Quantity = Quantity;
			ItemToMove.CurrentContainerUID = SourceItemPtr->CurrentContainerUID;
			ItemToMove.CurrentSlotID = TargetSlotID;
			
			// 添加新物品到AllItems
			AllItems.Add(ItemToMove.InstanceUID, ItemToMove);
			
			// 更新槽位引用
			TargetSlot.ItemInstanceUID = ItemToMove.InstanceUID;
			
			// 更新源物品数量
			SourceItemPtr->Quantity -= Quantity;
			
			UE_LOG(LogGaia, Log, TEXT("容器内部分移动: 原UID=%s, 新UID=%s, 数量=%d"), 
				*ItemUID.ToString(), *ItemToMove.InstanceUID.ToString(), Quantity);
		}
		else
		{
			// 完全移动：更新槽位ID
			int32 OldSlotID = SourceItemPtr->CurrentSlotID;
			SourceItemPtr->CurrentSlotID = TargetSlotID;
			
			// 清空原槽位
			int32 OldSlotIndex = Container->GetSlotIndexByID(OldSlotID);
			if (OldSlotIndex != INDEX_NONE)
			{
				Container->Slots[OldSlotIndex].ItemInstanceUID = FGuid();
			}
			
			// 更新新槽位
			TargetSlot.ItemInstanceUID = ItemUID;
			
			UE_LOG(LogGaia, Log, TEXT("容器内完全移动: UID=%s, 槽位 %d -> %d"), 
				*ItemUID.ToString(), OldSlotID, TargetSlotID);
		}
		
		Container->MarkDirty();
		
		Result.Result = EMoveItemResult::Success;
		Result.MovedQuantity = Quantity;
		Result.RemainingQuantity = 0;
		Result.TargetContainerUID = SourceItemPtr->CurrentContainerUID;
		
		UE_LOG(LogGaia, Log, TEXT("容器内移动成功: %s -> 槽位 %d"), *ItemUID.ToString(), TargetSlotID);
	}
	else
	{
		// 目标槽位有物品，需要处理
		FGaiaItemInstance TargetItem;
		if (FindItemByUID(TargetSlot.ItemInstanceUID, TargetItem))
		{
			return ProcessTargetSlotWithItem(ItemUID, TargetItem, SourceItemPtr->CurrentContainerUID, TargetSlotID, Quantity);
		}
	}
	
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::ProcessTargetSlotWithItem(const FGuid& ItemUID, const FGaiaItemInstance& TargetItem, const FGuid& TargetContainerUID, int32 TargetSlotID, int32 Quantity)
{
	FMoveItemResult Result;
	
	// 获取源物品
	FGaiaItemInstance SourceItem;
	if (!FindItemByUID(ItemUID, SourceItem))
	{
		Result.ErrorMessage = TEXT("无法找到源物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	// 检查是否为相同类型（尝试堆叠）
	if (SourceItem.ItemDefinitionID == TargetItem.ItemDefinitionID)
	{
		return TryStackItems(ItemUID, TargetItem.InstanceUID, Quantity);
	}
	
	// 检查目标物品是否有容器（尝试放入容器）
	if (TargetItem.HasContainer())
	{
		FMoveItemResult ContainerResult = TryMoveToContainer(ItemUID, TargetItem.InstanceUID, Quantity);
		if (ContainerResult.Result == EMoveItemResult::Success)
		{
			return ContainerResult;
		}
		// 如果放入容器失败，不交换位置，直接返回失败
		Result.ErrorMessage = TEXT("无法放入目标物品的容器");
		Result.Result = EMoveItemResult::ContainerRejected;
		return Result;
	}
	
	// 尝试交换位置
	if (CanSwapItems(ItemUID, TargetItem.InstanceUID))
	{
		return SwapItems(ItemUID, TargetItem.InstanceUID);
	}
	
	Result.ErrorMessage = TEXT("无法处理目标槽位");
	Result.Result = EMoveItemResult::Failed;
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::MoveItemAutoSlot(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 Quantity)
{
	FMoveItemResult Result;
	
	// 获取目标容器
	FGaiaContainerInstance* TargetContainer = Containers.Find(TargetContainerUID);
	if (!TargetContainer)
	{
		Result.ErrorMessage = TEXT("无法找到目标容器");
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	// 获取源物品
	FGaiaItemInstance SourceItem;
	if (!FindItemByUID(ItemUID, SourceItem))
	{
		Result.ErrorMessage = TEXT("无法找到源物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	// 查找空槽位
	int32 EmptySlotID = TargetContainer->FindEmptySlotID();
	if (EmptySlotID != INDEX_NONE)
	{
		return MoveToEmptySlot(ItemUID, TargetContainerUID, EmptySlotID, Quantity);
	}
	
	// 没有空槽位，尝试堆叠到现有物品
	for (const FGaiaSlotInfo& Slot : TargetContainer->Slots)
	{
		if (!Slot.IsEmpty())
		{
			FGaiaItemInstance TargetItem;
			if (FindItemByUID(Slot.ItemInstanceUID, TargetItem))
			{
				// 尝试堆叠
				if (SourceItem.ItemDefinitionID == TargetItem.ItemDefinitionID)
				{
					FMoveItemResult StackResult = TryStackItems(ItemUID, TargetItem.InstanceUID, Quantity);
					if (StackResult.IsSuccess())
					{
						return StackResult;
					}
				}
				
				// 尝试放入容器
				if (TargetItem.HasContainer())
				{
					FMoveItemResult ContainerResult = TryMoveToContainer(ItemUID, TargetItem.InstanceUID, Quantity);
					if (ContainerResult.Result == EMoveItemResult::Success)
					{
						return ContainerResult;
					}
				}
			}
		}
	}
	
	Result.ErrorMessage = TEXT("无法找到合适的位置移动物品");
	Result.Result = EMoveItemResult::ContainerFull;
	return Result;
}

//~END 移动辅助函数

//~BEGIN 查询辅助

TArray<FGaiaItemInstance> UGaiaInventorySubsystem::GetItemsInContainer(const FGuid& ContainerUID) const
{
	TArray<FGaiaItemInstance> Items;
	
	for (const auto& Pair : AllItems)
	{
		if (Pair.Value.CurrentContainerUID == ContainerUID)
		{
			Items.Add(Pair.Value);
		}
	}
	
	return Items;
}

TArray<FGaiaItemInstance> UGaiaInventorySubsystem::GetOrphanItems() const
{
	TArray<FGaiaItemInstance> Items;
	
	for (const auto& Pair : AllItems)
	{
		if (Pair.Value.IsOrphan())
		{
			Items.Add(Pair.Value);
		}
	}
	
	return Items;
}

int32 UGaiaInventorySubsystem::CountItemsByType(FName ItemDefID) const
{
	int32 Count = 0;
	
	for (const auto& Pair : AllItems)
	{
		if (Pair.Value.ItemDefinitionID == ItemDefID)
		{
			Count += Pair.Value.Quantity;
		}
	}
	
	return Count;
}

//~END 查询辅助

//~BEGIN 数据验证

bool UGaiaInventorySubsystem::ValidateDataIntegrity() const
{
	bool bIsValid = true;
	int32 ErrorCount = 0;
	
	UE_LOG(LogGaia, Log, TEXT("开始验证库存数据一致性..."));
	
	// 1. 验证物品位置信息
	for (const auto& ItemPair : AllItems)
	{
		const FGaiaItemInstance& Item = ItemPair.Value;
		
		// 如果物品在容器中
		if (Item.IsInContainer())
		{
			// 检查容器是否存在
			const FGaiaContainerInstance* Container = Containers.Find(Item.CurrentContainerUID);
			if (!Container)
			{
				UE_LOG(LogGaia, Error, TEXT("[验证失败] 物品 %s 引用的容器 %s 不存在！"),
					*Item.InstanceUID.ToString(), *Item.CurrentContainerUID.ToString());
				bIsValid = false;
				ErrorCount++;
				continue;
			}
			
			// 检查槽位是否存在
			int32 SlotIndex = Container->GetSlotIndexByID(Item.CurrentSlotID);
			if (SlotIndex == INDEX_NONE)
			{
				UE_LOG(LogGaia, Error, TEXT("[验证失败] 物品 %s 引用的槽位 %d 不存在！"),
					*Item.InstanceUID.ToString(), Item.CurrentSlotID);
				bIsValid = false;
				ErrorCount++;
				continue;
			}
			
			// 检查槽位引用是否匹配
			if (Container->Slots[SlotIndex].ItemInstanceUID != Item.InstanceUID)
			{
				UE_LOG(LogGaia, Error, TEXT("[验证失败] 数据不一致：物品 %s 认为自己在槽位 %d，但槽位引用的是 %s"),
					*Item.InstanceUID.ToString(), Item.CurrentSlotID,
					*Container->Slots[SlotIndex].ItemInstanceUID.ToString());
				bIsValid = false;
				ErrorCount++;
			}
		}
	}
	
	// 2. 验证槽位引用
	for (const auto& ContainerPair : Containers)
	{
		const FGaiaContainerInstance& Container = ContainerPair.Value;
		
		for (const FGaiaSlotInfo& Slot : Container.Slots)
		{
			if (!Slot.IsEmpty())
			{
				// 检查物品是否存在
				const FGaiaItemInstance* Item = AllItems.Find(Slot.ItemInstanceUID);
				if (!Item)
				{
					UE_LOG(LogGaia, Error, TEXT("[验证失败] 容器 %s 槽位 %d 引用的物品 %s 不存在！"),
						*Container.ContainerUID.ToString(), Slot.SlotID, *Slot.ItemInstanceUID.ToString());
					bIsValid = false;
					ErrorCount++;
					continue;
				}
				
				// 检查物品位置是否匹配
				if (Item->CurrentContainerUID != Container.ContainerUID)
				{
					UE_LOG(LogGaia, Error, TEXT("[验证失败] 容器 %s 槽位 %d 引用物品 %s，但物品认为自己在容器 %s！"),
						*Container.ContainerUID.ToString(), Slot.SlotID,
						*Item->InstanceUID.ToString(), *Item->CurrentContainerUID.ToString());
					bIsValid = false;
					ErrorCount++;
				}
				
				if (Item->CurrentSlotID != Slot.SlotID)
				{
					UE_LOG(LogGaia, Error, TEXT("[验证失败] 容器 %s 槽位 %d 引用物品 %s，但物品认为自己在槽位 %d！"),
						*Container.ContainerUID.ToString(), Slot.SlotID,
						*Item->InstanceUID.ToString(), Item->CurrentSlotID);
					bIsValid = false;
					ErrorCount++;
				}
			}
		}
	}
	
	if (bIsValid)
	{
		UE_LOG(LogGaia, Log, TEXT("库存数据一致性验证通过！"));
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("库存数据一致性验证失败！共发现 %d 个错误"), ErrorCount);
	}
	
	return bIsValid;
}

void UGaiaInventorySubsystem::RepairDataIntegrity()
{
	UE_LOG(LogGaia, Log, TEXT("开始修复库存数据一致性..."));
	
	int32 RepairCount = 0;
	
	// 策略：以槽位引用为准，修复物品位置
	for (auto& ContainerPair : Containers)
	{
		FGaiaContainerInstance& Container = ContainerPair.Value;
		
		for (const FGaiaSlotInfo& Slot : Container.Slots)
		{
			if (!Slot.IsEmpty())
			{
				if (FGaiaItemInstance* Item = AllItems.Find(Slot.ItemInstanceUID))
				{
					// 修复物品位置
					if (Item->CurrentContainerUID != Container.ContainerUID ||
						Item->CurrentSlotID != Slot.SlotID)
					{
						UE_LOG(LogGaia, Log, TEXT("[修复] 物品 %s 位置不一致，从 [容器:%s, 槽位:%d] 修复为 [容器:%s, 槽位:%d]"),
							*Item->InstanceUID.ToString(),
							*Item->CurrentContainerUID.ToString(), Item->CurrentSlotID,
							*Container.ContainerUID.ToString(), Slot.SlotID);
						
						Item->CurrentContainerUID = Container.ContainerUID;
						Item->CurrentSlotID = Slot.SlotID;
						RepairCount++;
					}
				}
				else
				{
					UE_LOG(LogGaia, Warning, TEXT("[修复] 槽位引用的物品 %s 不存在，无法修复"),
						*Slot.ItemInstanceUID.ToString());
				}
			}
		}
	}
	
	UE_LOG(LogGaia, Log, TEXT("库存数据一致性修复完成！共修复 %d 个问题"), RepairCount);
}

//~END 数据验证

