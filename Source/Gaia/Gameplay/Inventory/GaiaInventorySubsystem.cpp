#include "GaiaInventorySubsystem.h"
#include "GaiaLogChannels.h"
#include "DataRegistrySubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GaiaInventoryRPCComponent.h"

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
	WorldContainerAccessors.Empty();
	ContainerOwnerMap.Empty();
	
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
	// 注意：使用 IsStackable() 而不是 bStackable，因为带容器的物品强制不可堆叠
	if (ItemDef.IsStackable())
	{
		NewItem.Quantity = FMath::Clamp(Quantity, 1, ItemDef.MaxStackSize);
	}
	else
	{
		// 不可堆叠的物品，强制数量为1
		NewItem.Quantity = 1;
		
		// 如果用户请求创建多个不可堆叠物品，给出警告
		if (Quantity > 1)
		{
			UE_LOG(LogGaia, Warning, TEXT("创建物品: %s 不可堆叠（%s），数量从 %d 调整为 1"),
				*ItemDefID.ToString(),
				ItemDef.bHasContainer ? TEXT("带容器") : TEXT("定义设置"),
				Quantity);
		}
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

FGuid UGaiaInventorySubsystem::CreateContainerForPlayer(FName ContainerDefID, APlayerController* OwnerPlayerController)
{
	// 先创建容器
	FGuid ContainerUID = CreateContainer(ContainerDefID);
	
	// 如果创建成功且提供了所有者，注册所有者
	if (ContainerUID.IsValid() && OwnerPlayerController)
	{
		RegisterContainerOwner(OwnerPlayerController, ContainerUID);
		UE_LOG(LogGaia, Log, TEXT("容器 %s 已自动注册所有者: %s"), 
			*ContainerUID.ToString(), *OwnerPlayerController->GetName());
	}
	
	return ContainerUID;
}

//~END 实例创建

//~BEGIN 调试辅助

void UGaiaInventorySubsystem::SetItemDebugName(const FGuid& ItemUID, const FString& DebugName)
{
	if (FGaiaItemInstance* Item = AllItems.Find(ItemUID))
	{
		Item->DebugDisplayName = DebugName;
	}
}

void UGaiaInventorySubsystem::SetContainerDebugName(const FGuid& ContainerUID, const FString& DebugName)
{
	if (FGaiaContainerInstance* Container = Containers.Find(ContainerUID))
	{
		Container->DebugDisplayName = DebugName;
	}
}

//~END 调试辅助

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

	UE_LOG(LogGaia, Warning, TEXT("【移除物品】开始: 物品=%s [%s], 容器=%s [%s], 槽位=%d"), 
		*Item->GetDebugName(), *Item->GetShortUID(),
		*Container->GetDebugName(), *Container->GetShortUID(), Item->CurrentSlotID);
	
	// 清空槽位引用
	int32 SlotIndex = Container->GetSlotIndexByID(Item->CurrentSlotID);
	if (SlotIndex != INDEX_NONE)
	{
		UE_LOG(LogGaia, Warning, TEXT("【移除物品】清空槽位引用: 容器=%s, 槽位ID=%d, 槽位索引=%d, 原引用=%s"), 
			*Container->ContainerUID.ToString(), Item->CurrentSlotID, SlotIndex, 
			*Container->Slots[SlotIndex].ItemInstanceUID.ToString());
		
		Container->Slots[SlotIndex].ItemInstanceUID = FGuid();
		
		UE_LOG(LogGaia, Warning, TEXT("【移除物品】槽位引用已清空"));
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("【移除物品】无法找到槽位索引: SlotID=%d"), Item->CurrentSlotID);
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
	
	UE_LOG(LogGaia, Warning, TEXT("【删除物品】开始: ItemUID=%s"), *ItemUID.ToString());
	
	// 从全局池查找物品
	FGaiaItemInstance* Item = AllItems.Find(ItemUID);
	if (!Item)
	{
		UE_LOG(LogGaia, Warning, TEXT("【删除物品】失败: 物品不存在 - %s"), *ItemUID.ToString());
		return false;
	}
	
	UE_LOG(LogGaia, Warning, TEXT("【删除物品】物品信息: 容器=%s, 槽位=%d, 数量=%d"), 
		*Item->CurrentContainerUID.ToString(), Item->CurrentSlotID, Item->Quantity);
	
	// 如果物品在容器中，先从容器移除
	if (Item->IsInContainer())
	{
		UE_LOG(LogGaia, Warning, TEXT("【删除物品】调用RemoveItemFromContainer..."));
		RemoveItemFromContainer(ItemUID);
		UE_LOG(LogGaia, Warning, TEXT("【删除物品】RemoveItemFromContainer完成"));
	}
	else
	{
		UE_LOG(LogGaia, Warning, TEXT("【删除物品】物品处于游离状态，无需从容器移除"));
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
	
	UE_LOG(LogGaia, Warning, TEXT("【删除物品】完成: ItemUID=%s 已从AllItems中移除"), *ItemUID.ToString());
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
	
	// 检查物品定义是否允许堆叠（包含容器定义检查）
	if (!ItemDef.IsStackable())
	{
		if (ItemDef.bHasContainer)
		{
			Result.ErrorMessage = TEXT("带容器的物品不可堆叠");
			UE_LOG(LogGaia, Warning, TEXT("堆叠失败: 物品定义声明了容器功能，不可堆叠 - %s"), *SourceItem.ItemDefinitionID.ToString());
		}
		else
		{
			Result.ErrorMessage = TEXT("物品不可堆叠");
			UE_LOG(LogGaia, Warning, TEXT("堆叠失败: 物品定义设置为不可堆叠 - %s"), *SourceItem.ItemDefinitionID.ToString());
		}
		Result.Result = EMoveItemResult::StackLimitReached;
		return Result;
	}
	
	// 检查物品实例是否包含容器（运行时检查）
	// 即使定义允许堆叠，如果实例关联了容器，也不允许堆叠
	if (SourceItem.HasContainer() || TargetItem.HasContainer())
	{
		Result.ErrorMessage = TEXT("带容器的物品实例不可堆叠");
		Result.Result = EMoveItemResult::StackLimitReached;
		UE_LOG(LogGaia, Warning, TEXT("堆叠失败: 物品实例包含容器，不可堆叠 - 源HasContainer: %d, 目标HasContainer: %d"), 
			SourceItem.HasContainer(), TargetItem.HasContainer());
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
			UE_LOG(LogGaia, Warning, TEXT("【堆叠】源物品数量为0，准备删除: UID=%s, 容器=%s, 槽位=%d"), 
				*SourceItemUID.ToString(), 
				*SourceItemPtr->CurrentContainerUID.ToString(), 
				SourceItemPtr->CurrentSlotID);
			
			DestroyItem(SourceItemUID);
			
			UE_LOG(LogGaia, Warning, TEXT("【堆叠】源物品已删除: UID=%s"), *SourceItemUID.ToString());
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
	
	// ⭐ 关键修复：先从源容器移除槽位引用，但不删除物品
	FGuid SourceContainerUID = ItemToMove.CurrentContainerUID;
	int32 SourceSlotID = ItemToMove.CurrentSlotID;
	
	if (ItemToMove.IsInContainer())
	{
		UE_LOG(LogGaia, Warning, TEXT("【放入容器】清空源容器槽位引用: 容器=%s, 槽位=%d"),
			*SourceContainerUID.ToString(), SourceSlotID);
		
		// 清空源容器的槽位引用
		if (FGaiaContainerInstance* SourceContainer = Containers.Find(SourceContainerUID))
		{
			int32 SlotIndex = SourceContainer->GetSlotIndexByID(SourceSlotID);
			if (SlotIndex != INDEX_NONE)
			{
				SourceContainer->Slots[SlotIndex].ItemInstanceUID = FGuid();
				SourceContainer->MarkDirty();
				UE_LOG(LogGaia, Warning, TEXT("【放入容器】源槽位引用已清空"));
			}
		}
	}
	
	// 查找目标容器的空槽位
	FGaiaContainerInstance* TargetContainer = Containers.Find(ContainerItem.OwnedContainerUID);
	if (!TargetContainer)
	{
		Result.ErrorMessage = TEXT("目标容器不存在");
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	int32 EmptySlotID = TargetContainer->FindEmptySlotID();
	if (EmptySlotID == INDEX_NONE)
	{
		Result.ErrorMessage = TEXT("目标容器没有空槽位");
		Result.Result = EMoveItemResult::ContainerRejected;
		return Result;
	}
	
	// ⭐ 直接修改现有物品的位置信息（不创建新物品）
	if (FGaiaItemInstance* ItemPtr = AllItems.Find(ItemUID))
	{
		UE_LOG(LogGaia, Warning, TEXT("【放入容器】更新物品位置: %s -> 容器 %s 槽位 %d"),
			*ItemUID.ToString(), *ContainerItem.OwnedContainerUID.ToString(), EmptySlotID);
		
		// 更新物品位置
		ItemPtr->CurrentContainerUID = ContainerItem.OwnedContainerUID;
		ItemPtr->CurrentSlotID = EmptySlotID;
		
		// 更新目标容器的槽位引用
		int32 TargetSlotIndex = TargetContainer->GetSlotIndexByID(EmptySlotID);
		if (TargetSlotIndex != INDEX_NONE)
		{
			TargetContainer->Slots[TargetSlotIndex].ItemInstanceUID = ItemUID;
			TargetContainer->MarkDirty();
		}
		
		// 如果物品有容器，更新容器的父容器引用
		if (ItemPtr->HasContainer())
		{
			if (FGaiaContainerInstance* ItemContainer = Containers.Find(ItemPtr->OwnedContainerUID))
			{
				ItemContainer->ParentContainerUID = ContainerItem.OwnedContainerUID;
				UE_LOG(LogGaia, Log, TEXT("更新物品容器的父容器: %s -> %s"),
					*ItemPtr->OwnedContainerUID.ToString(), *ContainerItem.OwnedContainerUID.ToString());
			}
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
		Result.ErrorMessage = TEXT("无法找到源物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Error, TEXT("放入容器失败: 无法找到源物品 - %s"), *ItemUID.ToString());
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
		UE_LOG(LogGaia, Warning, TEXT("[交换检查] 无法找到物品: Item1=%d, Item2=%d"), 
			Item1 != nullptr, Item2 != nullptr);
		return false;
	}
	
	UE_LOG(LogGaia, Warning, TEXT("[交换检查] Item1: %s (容器: %s, 槽位: %d, HasContainer: %d)"),
		*Item1->ItemDefinitionID.ToString(),
		*Item1->CurrentContainerUID.ToString(),
		Item1->CurrentSlotID,
		Item1->HasContainer());
	
	UE_LOG(LogGaia, Warning, TEXT("[交换检查] Item2: %s (容器: %s, 槽位: %d, HasContainer: %d)"),
		*Item2->ItemDefinitionID.ToString(),
		*Item2->CurrentContainerUID.ToString(),
		Item2->CurrentSlotID,
		Item2->HasContainer());
	
	// 检查是否都在容器中
	if (!Item1->IsInContainer() || !Item2->IsInContainer())
	{
		UE_LOG(LogGaia, Warning, TEXT("[交换检查] ❌ 物品不在容器中"));
		return false;
	}
	
	// 如果在同一个容器中，直接允许交换（同容器内交换不会造成循环）
	if (Item1->CurrentContainerUID == Item2->CurrentContainerUID)
	{
		UE_LOG(LogGaia, Warning, TEXT("[交换检查] ✅ 同容器内交换，允许"));
		return true;
	}
	
	// 跨容器交换，需要检查是否可以添加到对方的容器
	UE_LOG(LogGaia, Warning, TEXT("[交换检查] 跨容器交换，检查是否可以互相添加..."));
	
	bool bCanAdd1To2 = CanAddItemToContainer(*Item1, Item2->CurrentContainerUID);
	bool bCanAdd2To1 = CanAddItemToContainer(*Item2, Item1->CurrentContainerUID);
	
	UE_LOG(LogGaia, Warning, TEXT("[交换检查] Item1 -> Item2容器: %d, Item2 -> Item1容器: %d"),
		bCanAdd1To2, bCanAdd2To1);
	
	return bCanAdd1To2 && bCanAdd2To1;
}

FMoveItemResult UGaiaInventorySubsystem::MoveToEmptySlot(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID, int32 Quantity)
{
	FMoveItemResult Result;
	
	// 获取源物品指针（直接从AllItems）
	FGaiaItemInstance* SourceItemPtr = AllItems.Find(ItemUID);
	if (!SourceItemPtr)
	{
		Result.ErrorMessage = TEXT("无法找到源物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	// 设置移动数量
	if (Quantity <= 0)
	{
		Quantity = SourceItemPtr->Quantity;
	}
	
	// 检查数量是否有效
	if (Quantity > SourceItemPtr->Quantity)
	{
		Result.ErrorMessage = TEXT("移动数量超过源物品数量");
		Result.Result = EMoveItemResult::Failed;
		return Result;
	}
	
	// 获取目标容器
	FGaiaContainerInstance* TargetContainer = Containers.Find(TargetContainerUID);
	if (!TargetContainer)
	{
		Result.ErrorMessage = TEXT("目标容器不存在");
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	// 找到目标槽位
	int32 TargetSlotIndex = TargetContainer->GetSlotIndexByID(TargetSlotID);
	if (TargetSlotIndex == INDEX_NONE)
	{
		Result.ErrorMessage = TEXT("目标槽位无效");
		Result.Result = EMoveItemResult::InvalidTarget;
		return Result;
	}
	
	// 如果是部分移动，创建新物品实例
	bool bIsPartialMove = (Quantity < SourceItemPtr->Quantity);
	
	if (bIsPartialMove)
	{
		// 部分移动：创建新物品
		FGaiaItemInstance NewItem = *SourceItemPtr;
		NewItem.InstanceUID = FGuid::NewGuid();
		NewItem.Quantity = Quantity;
		NewItem.CurrentContainerUID = TargetContainerUID;
		NewItem.CurrentSlotID = TargetSlotID;
		
		// 添加新物品到AllItems
		AllItems.Add(NewItem.InstanceUID, NewItem);
		
		// 更新目标槽位引用
		TargetContainer->Slots[TargetSlotIndex].ItemInstanceUID = NewItem.InstanceUID;
		
		// 减少源物品数量
		SourceItemPtr->Quantity -= Quantity;
		
		// 标记容器需要重新计算
		if (SourceItemPtr->IsInContainer())
		{
			if (FGaiaContainerInstance* SourceContainer = Containers.Find(SourceItemPtr->CurrentContainerUID))
			{
				SourceContainer->MarkDirty();
			}
		}
		TargetContainer->MarkDirty();
		
		UE_LOG(LogGaia, Log, TEXT("部分移动到空槽位: 原UID=%s (剩余%d), 新UID=%s (移动%d)"), 
			*ItemUID.ToString(), SourceItemPtr->Quantity, *NewItem.InstanceUID.ToString(), Quantity);
	}
	else
	{
		// 完全移动：直接更新物品位置
		// 先从源容器移除
		if (SourceItemPtr->IsInContainer())
		{
			FGaiaContainerInstance* SourceContainer = Containers.Find(SourceItemPtr->CurrentContainerUID);
			if (SourceContainer)
			{
				int32 SourceSlotIndex = SourceContainer->GetSlotIndexByID(SourceItemPtr->CurrentSlotID);
				if (SourceSlotIndex != INDEX_NONE)
				{
					SourceContainer->Slots[SourceSlotIndex].ItemInstanceUID = FGuid();
				}
				SourceContainer->MarkDirty();
			}
		}
		
		// 更新物品位置
		SourceItemPtr->CurrentContainerUID = TargetContainerUID;
		SourceItemPtr->CurrentSlotID = TargetSlotID;
		
		// 更新目标槽位引用
		TargetContainer->Slots[TargetSlotIndex].ItemInstanceUID = ItemUID;
		TargetContainer->MarkDirty();
		
		UE_LOG(LogGaia, Log, TEXT("完全移动到空槽位: UID=%s, 数量=%d"), 
			*ItemUID.ToString(), Quantity);
	}
	
	Result.Result = EMoveItemResult::Success;
	Result.MovedQuantity = Quantity;
	Result.RemainingQuantity = bIsPartialMove ? (SourceItemPtr->Quantity) : 0;
	Result.TargetContainerUID = TargetContainerUID;
	
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
	
	UE_LOG(LogGaia, Warning, TEXT("[容器内移动] 目标槽位%d: IsEmpty=%d, ItemUID=%s"),
		TargetSlotID, TargetSlot.IsEmpty(), *TargetSlot.ItemInstanceUID.ToString());
	
	if (TargetSlot.IsEmpty())
	{
		// 目标槽位为空，直接移动
		UE_LOG(LogGaia, Warning, TEXT("[容器内移动] ✅ 目标槽位为空，执行移动"));
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
		return Result; // ⭐ 修复：必须返回，避免被后面的代码覆盖
	}
	
	// 目标槽位有物品，需要处理
	UE_LOG(LogGaia, Warning, TEXT("[容器内移动] 目标槽位有物品，查找目标物品: %s"),
		*TargetSlot.ItemInstanceUID.ToString());
	
	FGaiaItemInstance TargetItem;
	if (FindItemByUID(TargetSlot.ItemInstanceUID, TargetItem))
	{
		UE_LOG(LogGaia, Warning, TEXT("[容器内移动] → 找到目标物品，调用 ProcessTargetSlotWithItem"));
		return ProcessTargetSlotWithItem(ItemUID, TargetItem, SourceItemPtr->CurrentContainerUID, TargetSlotID, Quantity);
	}
	
	// 无法找到目标物品（数据不一致）
	UE_LOG(LogGaia, Error, TEXT("[容器内移动] ❌ 无法找到目标物品！槽位引用的物品UID不存在: %s"),
		*TargetSlot.ItemInstanceUID.ToString());
	
	Result.ErrorMessage = TEXT("目标槽位数据异常：物品不存在");
	Result.Result = EMoveItemResult::Failed;
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::ProcessTargetSlotWithItem(const FGuid& ItemUID, const FGaiaItemInstance& TargetItem, const FGuid& TargetContainerUID, int32 TargetSlotID, int32 Quantity)
{
	FMoveItemResult Result;
	
	UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] 源物品: %s, 目标物品: %s (HasContainer: %d)"),
		*ItemUID.ToString(),
		*TargetItem.ItemDefinitionID.ToString(),
		TargetItem.HasContainer());
	
	// 获取源物品
	FGaiaItemInstance SourceItem;
	if (!FindItemByUID(ItemUID, SourceItem))
	{
		Result.ErrorMessage = TEXT("无法找到源物品");
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] ❌ 无法找到源物品"));
		return Result;
	}
	
	UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] 源物品: %s (HasContainer: %d)"),
		*SourceItem.ItemDefinitionID.ToString(),
		SourceItem.HasContainer());
	
	// 检查是否为相同类型（尝试堆叠）
	if (SourceItem.ItemDefinitionID == TargetItem.ItemDefinitionID)
	{
		UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] → 尝试堆叠"));
	}
	else
	{
		UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] 类型不同: 源=%s, 目标=%s"),
			*SourceItem.ItemDefinitionID.ToString(),
			*TargetItem.ItemDefinitionID.ToString());
	}
	
	if (SourceItem.ItemDefinitionID == TargetItem.ItemDefinitionID)
	{
		return TryStackItems(ItemUID, TargetItem.InstanceUID, Quantity);
	}
	
	// 检查目标物品是否有容器（尝试放入容器）
	if (TargetItem.HasContainer())
	{
		UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] → 目标物品有容器，尝试放入"));
		FMoveItemResult ContainerResult = TryMoveToContainer(ItemUID, TargetItem.InstanceUID, Quantity);
		if (ContainerResult.Result == EMoveItemResult::Success)
		{
			UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] ✅ 成功放入目标容器"));
			return ContainerResult;
		}
		// 如果放入容器失败，不交换位置，直接返回失败
		UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] ❌ 无法放入目标容器"));
		Result.ErrorMessage = TEXT("无法放入目标物品的容器");
		Result.Result = EMoveItemResult::ContainerRejected;
		return Result;
	}
	
	// 尝试交换位置
	UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] → 尝试交换位置"));
	if (CanSwapItems(ItemUID, TargetItem.InstanceUID))
	{
		UE_LOG(LogGaia, Warning, TEXT("[目标槽位有物品] ✅ 可以交换，执行交换"));
		return SwapItems(ItemUID, TargetItem.InstanceUID);
	}
	
	// 交换失败，给出详细原因
	FGaiaItemInstance SourceItemForCheck;
	if (FindItemByUID(ItemUID, SourceItemForCheck))
	{
		// 检查是否因为带容器物品导致的问题
		if (SourceItemForCheck.HasContainer() || TargetItem.HasContainer())
		{
			Result.ErrorMessage = TEXT("带容器的物品无法交换（可能因为：不允许嵌套容器、体积限制、或循环引用）");
		}
		else
		{
			Result.ErrorMessage = TEXT("无法交换：物品类型不同且不满足交换条件");
		}
	}
	else
	{
		Result.ErrorMessage = TEXT("无法处理目标槽位");
	}
	
	Result.Result = EMoveItemResult::Failed;
	UE_LOG(LogGaia, Warning, TEXT("移动失败: %s"), *Result.ErrorMessage);
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

//~BEGIN 网络/多人游戏支持

bool UGaiaInventorySubsystem::TryOpenWorldContainer(APlayerController* PlayerController, const FGuid& ContainerUID, FString& OutErrorMessage)
{
	if (!PlayerController)
	{
		OutErrorMessage = TEXT("无效的玩家控制器");
		return false;
	}

	// 检查容器是否存在
	if (!Containers.Contains(ContainerUID))
	{
		OutErrorMessage = TEXT("容器不存在");
		return false;
	}

	// 检查玩家是否有权限访问此容器
	FString PermissionError;
	if (!CanPlayerAccessContainer(PlayerController, ContainerUID, PermissionError))
	{
		OutErrorMessage = PermissionError;
		UE_LOG(LogGaia, Warning, TEXT("[网络] 玩家 %s 尝试打开容器 %s 失败: %s"),
			*PlayerController->GetName(), *ContainerUID.ToString(), *PermissionError);
		return false;
	}

	// 检查容器是否已被其他玩家打开
	if (WorldContainerAccessors.Find(ContainerUID))
	{
		if (APlayerController* ExistingAccessor = *WorldContainerAccessors.Find(ContainerUID))
		{
			if (ExistingAccessor && ExistingAccessor != PlayerController)
			{
				// 容器已被其他玩家占用
				FString OtherPlayerName = (ExistingAccessor)->GetPlayerState<APlayerState>() 
					? (ExistingAccessor)->GetPlayerState<APlayerState>()->GetPlayerName()
					: (ExistingAccessor)->GetName();
			
				OutErrorMessage = FString::Printf(TEXT("容器正被 %s 使用"), *OtherPlayerName);
			
				UE_LOG(LogGaia, Warning, TEXT("[网络] 玩家 %s 尝试打开容器 %s 失败: 已被 %s 占用"),
					*PlayerController->GetName(), *ContainerUID.ToString(), *OtherPlayerName);
			
				return false;
			}
		}
	}
	// 记录访问者
	WorldContainerAccessors.Add(ContainerUID, PlayerController);
	
	UE_LOG(LogGaia, Log, TEXT("[网络] 玩家 %s 打开世界容器: %s"),
		*PlayerController->GetName(), *ContainerUID.ToString());
	
	return true;
}

void UGaiaInventorySubsystem::CloseWorldContainer(APlayerController* PlayerController, const FGuid& ContainerUID)
{
	if (!PlayerController)
	{
		return;
	}

	// 检查是否是当前访问者
	if (APlayerController* CurrentAccessor = *WorldContainerAccessors.Find(ContainerUID))
	{
		if (CurrentAccessor == PlayerController)
		{
			// 移除访问记录
			WorldContainerAccessors.Remove(ContainerUID);
			
			UE_LOG(LogGaia, Log, TEXT("[网络] 玩家 %s 关闭世界容器: %s"),
				*PlayerController->GetName(), *ContainerUID.ToString());
		}
		else
		{
			UE_LOG(LogGaia, Warning, TEXT("[网络] 玩家 %s 尝试关闭容器 %s，但当前访问者是其他玩家"),
				*PlayerController->GetName(), *ContainerUID.ToString());
		}
	}
}

APlayerController* UGaiaInventorySubsystem::GetContainerAccessor(const FGuid& ContainerUID) const
{
	if (const TObjectPtr<APlayerController>* Accessor = WorldContainerAccessors.Find(ContainerUID))
	{
		return *Accessor;
	}
	return nullptr;
}

bool UGaiaInventorySubsystem::IsContainerOccupied(const FGuid& ContainerUID) const
{
	return WorldContainerAccessors.Contains(ContainerUID);
}

void UGaiaInventorySubsystem::BroadcastContainerUpdate(const FGuid& ContainerUID)
{
	// 收集所有需要通知的玩家
	TSet<APlayerController*> PlayersToNotify;
	
	// 1. 添加容器所有者
	TArray<APlayerController*> Owners = GetContainerOwners(ContainerUID);
	for (APlayerController* Owner : Owners)
	{
		if (Owner)
		{
			PlayersToNotify.Add(Owner);
		}
	}
	
	// 2. 添加正在访问的玩家
	if (APlayerController* Accessor = GetContainerAccessor(ContainerUID))
	{
		PlayersToNotify.Add(Accessor);
	}
	
	// 3. 通知所有相关玩家刷新库存
	UE_LOG(LogGaia, Warning, TEXT("[网络广播] ⭐ 容器 %s 开始通知 %d 个玩家"), 
		*ContainerUID.ToString(), PlayersToNotify.Num());
	
	for (APlayerController* PC : PlayersToNotify)
	{
		if (!PC) continue;
		
		UE_LOG(LogGaia, Warning, TEXT("[网络广播] 尝试通知玩家: %s"), *PC->GetName());
		
		// 查找玩家的 RPC 组件
		UActorComponent* Component = PC->GetComponentByClass(UGaiaInventoryRPCComponent::StaticClass());
		if (UGaiaInventoryRPCComponent* RPCComp = Cast<UGaiaInventoryRPCComponent>(Component))
		{
			UE_LOG(LogGaia, Warning, TEXT("[网络广播] ✅ 找到RPC组件，调用刷新: %p"), RPCComp);
			
			// 通知客户端刷新库存数据
			// 这会在 ServerRequestRefreshInventory_Implementation 中执行
			// 因为我们在服务器端，直接调用 Implementation 版本
			RPCComp->ServerRequestRefreshInventory_Implementation();
		}
		else
		{
			UE_LOG(LogGaia, Error, TEXT("[网络广播] ❌ 未找到RPC组件"));
		}
	}
}

TArray<APlayerController*> UGaiaInventorySubsystem::GetContainerOwners(const FGuid& ContainerUID) const
{
	TArray<APlayerController*> Owners;
	
	// 查找容器的所有者
	if (const TObjectPtr<APlayerController>* Owner = ContainerOwnerMap.Find(ContainerUID))
	{
		if (*Owner)
		{
			Owners.Add(*Owner);
		}
	}
	
	return Owners;
}

void UGaiaInventorySubsystem::RegisterContainerOwner(APlayerController* PlayerController, const FGuid& ContainerUID)
{
	if (!PlayerController || !ContainerUID.IsValid())
	{
		return;
	}
	
	// 设置容器的所有者
	ContainerOwnerMap.Add(ContainerUID, PlayerController);
	
	// 如果容器当前是World类型，自动升级为Private类型
	FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (Container && Container->OwnershipType == EContainerOwnershipType::World)
	{
		Container->OwnershipType = EContainerOwnershipType::Private;
		UE_LOG(LogGaia, Verbose, TEXT("[网络] 容器 %s 已自动设置为Private类型"), 
			*ContainerUID.ToString());
	}
	
	UE_LOG(LogGaia, Verbose, TEXT("[网络] 注册容器所有者: 玩家=%s, 容器=%s"),
		*PlayerController->GetName(), *ContainerUID.ToString());
}

void UGaiaInventorySubsystem::UnregisterContainerOwner(APlayerController* PlayerController, const FGuid& ContainerUID)
{
	if (!PlayerController || !ContainerUID.IsValid())
	{
		return;
	}
	
	// 检查是否是当前所有者
	if (const TObjectPtr<APlayerController>* CurrentOwner = ContainerOwnerMap.Find(ContainerUID))
	{
		if (*CurrentOwner == PlayerController)
		{
			ContainerOwnerMap.Remove(ContainerUID);
			
			UE_LOG(LogGaia, Verbose, TEXT("[网络] 注销容器所有者: 玩家=%s, 容器=%s"),
				*PlayerController->GetName(), *ContainerUID.ToString());
		}
	}
}

// ========================================
// 权限和所有权管理
// ========================================

FGuid UGaiaInventorySubsystem::GetPlayerUID(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return FGuid();
	}

	// 使用PlayerState的UniqueId来获取全局唯一标识
	if (APlayerState* PlayerState = PlayerController->GetPlayerState<APlayerState>())
	{
		// 将UniqueId转换为FGuid
		// 注意：UniqueNetId可能是不同类型，这里使用ToString后生成一个确定性的GUID
		FString UniqueIdString = PlayerState->GetUniqueId().ToString();
		
		// 生成一个基于字符串的确定性GUID
		// 这确保同一个玩家总是得到相同的UID
		uint32 Hash = GetTypeHash(UniqueIdString);
		return FGuid(Hash, Hash >> 16, Hash >> 8, Hash);
	}

	// 回退方案：如果没有PlayerState，使用NetPlayerIndex
	int32 NetIndex = PlayerController->NetPlayerIndex;
	if (NetIndex >= 0)
	{
		return FGuid(NetIndex, 0, 0, 0);
	}

	// 最后的回退：生成一个随机UID（仅在本地单机模式下）
	UE_LOG(LogGaia, Warning, TEXT("[权限] 无法为玩家 %s 获取唯一UID，生成临时UID"), 
		*PlayerController->GetName());
	return FGuid::NewGuid();
}

void UGaiaInventorySubsystem::SetContainerOwnershipType(const FGuid& ContainerUID, EContainerOwnershipType OwnershipType)
{
	if (FGaiaContainerInstance* Container = Containers.Find(ContainerUID))
	{
		Container->OwnershipType = OwnershipType;
		UE_LOG(LogGaia, Log, TEXT("[所有权] 设置容器 %s 的所有权类型为: %d"), 
			*ContainerUID.ToString(), static_cast<int32>(OwnershipType));
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[所有权] 容器 %s 不存在，无法设置所有权类型"), 
			*ContainerUID.ToString());
	}
}

EContainerOwnershipType UGaiaInventorySubsystem::GetContainerOwnershipType(const FGuid& ContainerUID) const
{
	if (const FGaiaContainerInstance* Container = Containers.Find(ContainerUID))
	{
		return Container->OwnershipType;
	}
	
	UE_LOG(LogGaia, Warning, TEXT("[所有权] 容器 %s 不存在，返回默认类型 World"), 
		*ContainerUID.ToString());
	return EContainerOwnershipType::World;
}

void UGaiaInventorySubsystem::AuthorizePlayerAccess(const FGuid& ContainerUID, APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		UE_LOG(LogGaia, Error, TEXT("[授权] PlayerController 无效"));
		return;
	}

	FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (!Container)
	{
		UE_LOG(LogGaia, Error, TEXT("[授权] 容器 %s 不存在"), *ContainerUID.ToString());
		return;
	}

	// 只有 Shared 类型容器才需要授权列表
	if (Container->OwnershipType != EContainerOwnershipType::Shared)
	{
		UE_LOG(LogGaia, Warning, TEXT("[授权] 容器 %s 不是Shared类型，无需授权"), 
			*ContainerUID.ToString());
		return;
	}

	FGuid PlayerUID = GetPlayerUID(PlayerController);
	Container->AddAuthorizedPlayer(PlayerUID);

	UE_LOG(LogGaia, Log, TEXT("[授权] 玩家 %s (UID: %s) 已被授权访问容器 %s"), 
		*PlayerController->GetName(), *PlayerUID.ToString(), *ContainerUID.ToString());
}

void UGaiaInventorySubsystem::RevokePlayerAccess(const FGuid& ContainerUID, APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		UE_LOG(LogGaia, Error, TEXT("[取消授权] PlayerController 无效"));
		return;
	}

	FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (!Container)
	{
		UE_LOG(LogGaia, Error, TEXT("[取消授权] 容器 %s 不存在"), *ContainerUID.ToString());
		return;
	}

	FGuid PlayerUID = GetPlayerUID(PlayerController);
	Container->RemoveAuthorizedPlayer(PlayerUID);

	UE_LOG(LogGaia, Log, TEXT("[取消授权] 玩家 %s (UID: %s) 已被取消访问容器 %s 的授权"), 
		*PlayerController->GetName(), *PlayerUID.ToString(), *ContainerUID.ToString());
}

bool UGaiaInventorySubsystem::CanPlayerAccessContainer(APlayerController* PlayerController, const FGuid& ContainerUID, FString& OutErrorMessage) const
{
	if (!PlayerController)
	{
		OutErrorMessage = TEXT("玩家控制器无效");
		return false;
	}

	const FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (!Container)
	{
		OutErrorMessage = FString::Printf(TEXT("容器 %s 不存在"), *ContainerUID.ToString());
		return false;
	}

	FGuid PlayerUID = GetPlayerUID(PlayerController);

	// 根据容器类型检查权限
	switch (Container->OwnershipType)
	{
	case EContainerOwnershipType::Private:
		{
			// 私有容器：只有所有者可以访问
			const TObjectPtr<APlayerController>* Owner = ContainerOwnerMap.Find(ContainerUID);
			if (!Owner || *Owner != PlayerController)
			{
				OutErrorMessage = TEXT("这是私有容器，只有所有者可以访问");
				return false;
			}
		}
		break;

	case EContainerOwnershipType::World:
		// 世界容器：任何人都可以访问（但受独占访问限制）
		break;

	case EContainerOwnershipType::Shared:
		{
			// 共享容器：所有者或授权玩家可以访问
			const TObjectPtr<APlayerController>* Owner = ContainerOwnerMap.Find(ContainerUID);
			bool bIsOwner = Owner && (*Owner == PlayerController);
			bool bIsAuthorized = Container->IsPlayerAuthorized(PlayerUID);

			if (!bIsOwner && !bIsAuthorized)
			{
				OutErrorMessage = TEXT("你没有权限访问此共享容器");
				return false;
			}
		}
		break;

	case EContainerOwnershipType::Trade:
		{
			// 交易容器：需要特殊逻辑（TODO: 实现交易系统时完善）
			UE_LOG(LogGaia, Warning, TEXT("[权限] 交易容器权限检查暂未实现"));
		}
		break;
	}

	OutErrorMessage = TEXT("");
	return true;
}

bool UGaiaInventorySubsystem::CanPlayerAccessItem(APlayerController* PlayerController, const FGuid& ItemUID, FString& OutErrorMessage) const
{
	if (!PlayerController)
	{
		OutErrorMessage = TEXT("玩家控制器无效");
		return false;
	}

	const FGaiaItemInstance* Item = AllItems.Find(ItemUID);
	if (!Item)
	{
		OutErrorMessage = FString::Printf(TEXT("物品 %s 不存在"), *ItemUID.ToString());
		return false;
	}

	// 如果物品在容器中，检查容器访问权限
	if (Item->IsInContainer())
	{
		FString ContainerError;
		if (!CanPlayerAccessContainer(PlayerController, Item->CurrentContainerUID, ContainerError))
		{
			OutErrorMessage = FString::Printf(TEXT("无法访问物品所在的容器: %s"), *ContainerError);
			return false;
		}
	}
	// 如果物品是游离状态（掉落、装备等），这里可以添加额外的检查
	else
	{
		// TODO: 添加游离物品的权限检查（例如：检查物品是否在玩家附近）
	}

	OutErrorMessage = TEXT("");
	return true;
}

FContainerUIDebugInfo UGaiaInventorySubsystem::GetContainerDebugInfo(const FGuid& ContainerUID)
{
	FContainerUIDebugInfo DebugInfo;
	DebugInfo.ContainerUID = ContainerUID;
	
	// 查找容器
	FGaiaContainerInstance Container;
	if (!FindContainerByUID(ContainerUID, Container))
	{
		DebugInfo.SlotUsage = TEXT("容器不存在");
		return DebugInfo;
	}
	
	// 基础信息
	DebugInfo.ContainerDefID = Container.ContainerDefinitionID;
	DebugInfo.OwnershipType = Container.OwnershipType;
	
	// 所有者信息
	if (ContainerOwnerMap.Contains(ContainerUID))
	{
		if (APlayerController* Owner = ContainerOwnerMap[ContainerUID])
		{
			DebugInfo.OwnerPlayerName = Owner->GetName();
		}
	}
	
	// 授权玩家列表
	for (const FGuid& PlayerUID : Container.AuthorizedPlayerUIDs)
	{
		DebugInfo.AuthorizedPlayerNames.Add(PlayerUID.ToString());
	}
	
	// 当前访问者
	if (WorldContainerAccessors.Contains(ContainerUID))
	{
		if (APlayerController* Accessor = WorldContainerAccessors[ContainerUID])
		{
			DebugInfo.CurrentAccessorName = Accessor->GetName();
		}
	}
	
	// 获取容器定义（用于获取限制值）
	FGaiaContainerDefinition ContainerDef;
	bool bFoundDef = GetContainerDefinition(Container.ContainerDefinitionID, ContainerDef);
	
	// 槽位使用情况
	int32 UsedSlots = 0;
	TArray<FGaiaItemInstance> ItemsInContainer = GetItemsInContainer(ContainerUID);
	UsedSlots = ItemsInContainer.Num();
	
	int32 MaxSlots = bFoundDef ? ContainerDef.SlotCount : Container.Slots.Num();
	
	DebugInfo.SlotUsage = FString::Printf(TEXT("%d / %d (%.1f%%)"),
		UsedSlots,
		MaxSlots,
		MaxSlots > 0 ? (float)UsedSlots / MaxSlots * 100.0f : 0.0f
	);
	
	// 重量和体积信息
	int32 TotalWeight = 0;
	int32 TotalVolume = 0;
	
	for (const FGaiaItemInstance& Item : ItemsInContainer)
	{
		FGaiaItemDefinition ItemDef;
		if (GetItemDefinition(Item.ItemDefinitionID, ItemDef))
		{
			TotalWeight += ItemDef.ItemWeight * Item.Quantity;
			TotalVolume += ItemDef.ItemVolume * Item.Quantity;
		}
	}
	
	// 注意：当前项目没有实现MaxWeight限制，只有MaxVolume
	int32 MaxVolume = bFoundDef ? ContainerDef.MaxVolume : 0;
	bool bVolumeEnabled = bFoundDef && ContainerDef.bEnableVolumeLimit;
	
	DebugInfo.WeightInfo = FString::Printf(TEXT("%d (总重量)"), TotalWeight);
	
	DebugInfo.VolumeInfo = bVolumeEnabled
		? FString::Printf(TEXT("%d / %d (%.1f%%)"),
			TotalVolume,
			MaxVolume,
			MaxVolume > 0 ? (float)TotalVolume / MaxVolume * 100.0f : 0.0f
		)
		: FString::Printf(TEXT("%d (无限制)"), TotalVolume);
	
	// 物品列表
	for (const FGaiaItemInstance& Item : ItemsInContainer)
	{
		FString ItemStr = FString::Printf(TEXT("槽位%d: %s x%d (UID: %s)"),
			Item.CurrentSlotID,
			*Item.ItemDefinitionID.ToString(),
			Item.Quantity,
			*Item.InstanceUID.ToString().Left(8) // 只显示前8位
		);
		DebugInfo.ItemList.Add(ItemStr);
	}
	
	return DebugInfo;
}

//~END 网络/多人游戏支持

