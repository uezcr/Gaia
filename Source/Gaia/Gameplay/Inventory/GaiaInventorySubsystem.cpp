#include "GaiaInventorySubsystem.h"
#include "GaiaLogChannels.h"
#include "DataRegistrySubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GaiaInventoryRPCComponent.h"
#include "BehaviorTree/Tasks/BTTask_SetKeyValue.h"

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
		FGuid ContainerUID = CreateContainerInstance(ItemDef.ContainerDefinitionID);
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
	
	// 添加物品到全局池
	AllItems.Add(NewItem.InstanceUID, NewItem);
	
	UE_LOG(LogGaia, Log, TEXT("创建物品实例: %s, UID: %s, 数量: %d"), 
		*ItemDefID.ToString(), *NewItem.InstanceUID.ToString(), NewItem.Quantity);
	
	return NewItem;
}

FGuid UGaiaInventorySubsystem::CreateContainerInstance(FName ContainerDefID)
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

FAddItemResult UGaiaInventorySubsystem::TryAddItemToContainer(const FGuid& ItemUID, const FGuid& ContainerUID)
{
	// 检查物品是否存在
	FGaiaItemInstance* Item = AllItems.Find(ItemUID);
	if (!Item)
	{
		return FAddItemResult::Failure(FString::Printf(
		TEXT("物品不存在 (ItemUID: %s)"), *ItemUID.ToString()));
	}
	
	// 检查容器是否存在
	FGaiaContainerInstance* Container = Containers.Find(ContainerUID);
	if (!Container)
	{
		return FAddItemResult::Failure(FString::Printf(
		TEXT("容器不存在 (ContainerUID: %s)"), *ContainerUID.ToString()));
	}

	FAddItemResult AddItemResult = CanAddItemToContainer(Item, Container);
	if (!AddItemResult.IsSuccess())
	{
		return FAddItemResult::Failure(FString::Printf(
		TEXT("添加失败 (%s)"), *AddItemResult.ErrorMessage));
	}
	
	// 所有检查通过，执行添加（传递指针，避免重复查找）
	if (AddItemToContainer(Item, Container, AddItemResult.SlotID))
	{
		return FAddItemResult::Success(AddItemResult.SlotID);
	}
	
	return FAddItemResult::Failure(TEXT("添加物品失败（未知原因）"));
}

bool UGaiaInventorySubsystem::AddItemToContainer(FGaiaItemInstance* Item, FGaiaContainerInstance* Container, const int32 SlotID)
{
	// 参数已在调用者处验证，此处使用断言确保契约
	check(Item);
	check(Container);
	check(SlotID != INDEX_NONE);  // 确保槽位有效
	
	// 更新物品位置信息
	Item->CurrentContainerUID = Container->ContainerUID;
	Item->CurrentSlotID = SlotID;
	// 更新槽位信息
	int32 SlotIndex = Container->GetSlotIndexByID(SlotID);
	if (SlotIndex == INDEX_NONE)
	{
		UE_LOG(LogGaia, Error, TEXT("[AddItemToContainer] 无法找到槽位索引，SlotID: %d"), SlotID);
		return false;
	}
	Container->Slots[SlotIndex].ItemInstanceUID = Item->InstanceUID;
	// 如果物品有容器，更新容器的父容器
	if (Item->HasContainer())
	{
		if (FGaiaContainerInstance* ItemContainer = Containers.Find(Item->OwnedContainerUID))
		{
			ItemContainer->ParentContainerUID = Container->ContainerUID;
		}
	}
	// 标记需要重新计算
	Container->MarkDirty();
	UE_LOG(LogGaia, Verbose, TEXT("[AddItemToContainer] 添加成功: 物品 %s -> 容器 %s 槽位 %d"), 
		*Item->InstanceUID.ToString(),
		*Container->ContainerUID.ToString(),
		SlotID);
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

FAddItemResult UGaiaInventorySubsystem::CanAddItemToContainer(FGaiaItemInstance* Item, FGaiaContainerInstance* Container) const
{
	check(Item);
	check(Container);
	FAddItemResult Result;
	// 获取容器定义
	FGaiaContainerDefinition ContainerDef;
	if (!GetContainerDefinition(Container->ContainerDefinitionID, ContainerDef))
	{
		Result.ResultType = EMoveItemResult::InvalidDefinition;
		Result.ErrorMessage = FString::Printf(TEXT("无法获取容器定义 (ContainerDefID: %s)"),
			*Container->ContainerDefinitionID.ToString());
		return Result;
	}
	
	// 获取物品定义
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(Item->ItemDefinitionID, ItemDef))
	{
		Result.ResultType = EMoveItemResult::InvalidDefinition;
		Result.ErrorMessage = FString::Printf(TEXT("无法获取物品定义 (ItemDefID: %s)"),
			*Item->ItemDefinitionID.ToString());
		return Result;
	}
	
	// 检查物品标签是否允许
	if (ContainerDef.AllowedItemTags.IsEmpty())
	{
		Result.ResultType = EMoveItemResult::TypeMismatch;
		Result.ErrorMessage = FString::Printf(TEXT("容器没有任何标签 (%s-%s)"),
			*ContainerDef.ContainerName.ToString(),
			*Container->GetShortUID());
		return Result;
	}
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
		Result.ResultType = EMoveItemResult::TypeMismatch;
		Result.ErrorMessage = FString::Printf(TEXT("物品标签不匹配容器允许的标签 (ItemDefID: %s, 物品标签: %s, 允许标签: %s)"),
			*Item->ItemDefinitionID.ToString(),
			*ItemDef.ItemTags.ToStringSimple(),
			*ContainerDef.AllowedItemTags.ToStringSimple());
		return Result;
	}
	
	// 如果是容器物品，进行额外检查
	if (ItemDef.bHasContainer)
	{
		// 检查目标容器是否允许嵌套容器
		if (!ContainerDef.bAllowNestedContainers)
		{
			Result.ResultType = EMoveItemResult::ContainerRejected;
			Result.ErrorMessage = FString::Printf(TEXT("目标容器不允许嵌套 (%s-%s)"),
				*ContainerDef.ContainerName.ToString(),
				*Container->ContainerUID.ToString());
			return Result;
		}
		
		// 检查循环引用
		if (Item->HasContainer())
		{
			if (WouldCreateCycle(Item->OwnedContainerUID, Container->ContainerUID))
			{
				Result.ResultType = EMoveItemResult::CycleDetected;
				Result.ErrorMessage = FString::Printf(TEXT("会造成容器循环引用 (物品容器UID: %s, 目标容器UID: %s)"),
					*Item->OwnedContainerUID.ToString(),
					*Container->ContainerUID.ToString());
				return Result;
			}
		}
	}
	// 检查体积限制
	if (ContainerDef.bEnableVolumeLimit)
	{
		int32 UsedVolume = GetContainerUsedVolume(Container->ContainerUID);
		int32 ItemVolume = ItemDef.ItemVolume * Item->Quantity;
		int32 AvailableVolume = ContainerDef.MaxVolume - UsedVolume;
		if (UsedVolume + ItemVolume > ContainerDef.MaxVolume)
		{
			Result.ResultType = EMoveItemResult::VolumeExceeded;
			Result.ErrorMessage = FString::Printf(TEXT("容器体积不足 (需要: %d, 剩余: %d, 总容量: %d)"),
	          ItemDef.ItemVolume,
	          AvailableVolume,
	          ContainerDef.MaxVolume);
			return Result;
		}
	}

	// 检查是否有空槽位
	Result.SlotID = Container->FindEmptySlotID();
	if (Result.SlotID == INDEX_NONE)
	{
		Result.ResultType = EMoveItemResult::ContainerFull;
		Result.ErrorMessage = FString::Printf(TEXT("容器没有空槽位 (%s-%s)"),
			*ContainerDef.ContainerName.ToString(),
			*Container->GetShortUID());
		return Result;
	}
	
	Result.ResultType = EMoveItemResult::Success;
	return Result;
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

int32 UGaiaInventorySubsystem::GetItemTotalVolume(const FGaiaItemInstance& Item)
{
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(Item.ItemDefinitionID, ItemDef))
	{
		return 0;
	}
	return ItemDef.ItemVolume * Item.Quantity;
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

FMoveItemResult UGaiaInventorySubsystem::TryMoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID, int32 Quantity)
{
	FMoveItemResult Result;
	
	UE_LOG(LogGaia, Log, TEXT("[TryMoveItem] 开始移动物品: ItemUID=%s, TargetContainer=%s, TargetSlot=%d, Quantity=%d"), 
		*ItemUID.ToString(), *TargetContainerUID.ToString(), TargetSlotID, Quantity);
	
	// 检查源物品是否存在
	FGaiaItemInstance* SourceItem = AllItems.Find(ItemUID);
	if (!SourceItem)
	{
		Result.ErrorMessage = FString::Printf(TEXT("无法找到源物品 (ItemUID: %s)"), *ItemUID.ToString());
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Warning, TEXT("[TryMoveItem] %s"), *Result.ErrorMessage);
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("[TryMoveItem] 找到源物品: %s (数量: %d)"), 
		*SourceItem->ItemDefinitionID.ToString(), SourceItem->Quantity);
	
	// 检查目标容器是否存在
	FGaiaContainerInstance* TargetContainer = Containers.Find(TargetContainerUID);
	if (!TargetContainer)
	{
		Result.ErrorMessage = FString::Printf(TEXT("无法找到目标容器 (ContainerUID: %s)"), *TargetContainerUID.ToString());
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Warning, TEXT("[TryMoveItem] %s"), *Result.ErrorMessage);
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("[TryMoveItem] 找到目标容器: %s (槽位数: %d)"), 
		*TargetContainer->ContainerDefinitionID.ToString(), TargetContainer->Slots.Num());
	
	// 设置默认数量（移动全部）
	if (Quantity <= 0)
	{
		Quantity = SourceItem->Quantity;
		UE_LOG(LogGaia, Log, TEXT("[TryMoveItem] 设置移动数量为全部: %d"), Quantity);
	}
	
	// 检查数量是否有效
	if (Quantity > SourceItem->Quantity)
	{
		Result.ErrorMessage = FString::Printf(TEXT("移动数量 (%d) 超过源物品数量 (%d)"), Quantity, SourceItem->Quantity);
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Warning, TEXT("[TryMoveItem] %s"), *Result.ErrorMessage);
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("[TryMoveItem] 移动数量验证通过: %d"), Quantity);
	
	// 检查源物品是否在容器中
	if (!SourceItem->IsInContainer())
	{
		Result.ErrorMessage = FString::Printf(TEXT("源物品不在任何容器中 (ItemUID: %s)"), *ItemUID.ToString());
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("[TryMoveItem] %s"), *Result.ErrorMessage);
		return Result;
	}
	
	UE_LOG(LogGaia, Log, TEXT("[TryMoveItem] 找到源物品位置: Container=%s, SlotID=%d"), 
		*SourceItem->CurrentContainerUID.ToString(), SourceItem->CurrentSlotID);
	
	// 执行移动（传递指针，避免重复查找）
	return MoveItem(SourceItem, TargetContainer, TargetSlotID, Quantity);
}

FMoveItemResult UGaiaInventorySubsystem::MoveItem(FGaiaItemInstance* Item, FGaiaContainerInstance* TargetContainer, int32 TargetSlotID, int32 Quantity)
{
	// 参数已在调用者处验证，此处使用断言确保契约
	check(Item);
	check(TargetContainer);
	check(Item->IsInContainer());
	check(Quantity > 0 && Quantity <= Item->Quantity);
	
	FMoveItemResult Result;
	
	// 获取源容器
	FGaiaContainerInstance* SourceContainer = Containers.Find(Item->CurrentContainerUID);
	if (!SourceContainer)
	{
		Result.ErrorMessage = FString::Printf(TEXT("无法找到源容器 (ContainerUID: %s)"), *Item->CurrentContainerUID.ToString());
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("[MoveItem] %s"), *Result.ErrorMessage);
		return Result;
	}
	
	// 如果是同一个容器内的移动，需要特殊处理
	if (Item->CurrentContainerUID == TargetContainer->ContainerUID)
	{
		UE_LOG(LogGaia, Verbose, TEXT("[MoveItem] 执行容器内移动"));
		return MoveItemWithinContainer(Item, TargetSlotID, Quantity);
	}
	
	UE_LOG(LogGaia, Verbose, TEXT("[MoveItem] 执行跨容器移动: %s -> %s"), 
		*Item->CurrentContainerUID.ToString(), *TargetContainer->ContainerUID.ToString());
	
	// 检查目标槽位
	if (TargetSlotID >= 0)
	{
		UE_LOG(LogGaia, Verbose, TEXT("[MoveItem] 指定目标槽位: %d"), TargetSlotID);
		
		// 指定槽位移动
		int32 TargetSlotIndex = TargetContainer->GetSlotIndexByID(TargetSlotID);
		if (TargetSlotIndex == INDEX_NONE)
		{
			Result.ErrorMessage = FString::Printf(TEXT("目标槽位无效 (SlotID: %d)"), TargetSlotID);
			Result.Result = EMoveItemResult::InvalidTarget;
			UE_LOG(LogGaia, Warning, TEXT("[MoveItem] %s"), *Result.ErrorMessage);
			return Result;
		}
		
		FGaiaSlotInfo& TargetSlot = TargetContainer->Slots[TargetSlotIndex];
		
		if (TargetSlot.IsEmpty())
		{
			UE_LOG(LogGaia, Verbose, TEXT("[MoveItem] 目标槽位为空，执行直接移动"));
			return MoveToEmptySlot(Item, TargetContainer, TargetSlotID, Quantity);
		}
		else
		{
			// 目标槽位有物品，需要处理
			UE_LOG(LogGaia, Verbose, TEXT("[MoveItem] 目标槽位有物品: %s"), *TargetSlot.ItemInstanceUID.ToString());
			
			FGaiaItemInstance* TargetItem = AllItems.Find(TargetSlot.ItemInstanceUID);
			if (!TargetItem)
			{
				Result.ErrorMessage = FString::Printf(TEXT("无法找到目标槽位中的物品 (ItemUID: %s)"), *TargetSlot.ItemInstanceUID.ToString());
				Result.Result = EMoveItemResult::Failed;
				UE_LOG(LogGaia, Error, TEXT("[MoveItem] %s"), *Result.ErrorMessage);
				return Result;
			}
			
			UE_LOG(LogGaia, Verbose, TEXT("[MoveItem] 找到目标物品: %s (数量: %d)"), 
				*TargetItem->ItemDefinitionID.ToString(), TargetItem->Quantity);
			
			return ProcessTargetSlotWithItem(Item, TargetItem, TargetContainer, TargetSlotID, Quantity);
		}
	}
	else
	{
		UE_LOG(LogGaia, Verbose, TEXT("[MoveItem] 自动分配槽位移动"));
		return MoveItemAutoSlot(Item, TargetContainer, Quantity);
	}
}

FMoveItemResult UGaiaInventorySubsystem::QuickMoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID)
{
	return TryMoveItem(ItemUID, TargetContainerUID, TargetSlotID, -1); // -1 表示移动全部
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
	
	return TryMoveItem(ItemUID, TargetContainerUID, TargetSlotID, Quantity);
}

//~END 物品移动

//~BEGIN 移动辅助函数

FMoveItemResult UGaiaInventorySubsystem::StackItems(FGaiaItemInstance* SourceItem, FGaiaItemInstance* TargetItem, int32 Quantity)
{
	// 参数已在调用者处验证，此处使用断言确保契约
	check(SourceItem);
	check(TargetItem);
	check(Quantity > 0);
	check(Quantity <= SourceItem->Quantity);
	
	FMoveItemResult Result;
	
	// 获取物品定义（用于获取堆叠上限）
	FGaiaItemDefinition ItemDef;
	if (!GetItemDefinition(SourceItem->ItemDefinitionID, ItemDef))
	{
		Result.ErrorMessage = TEXT("无法获取物品定义");
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("[StackItems] 无法获取物品定义 - %s"), *SourceItem->ItemDefinitionID.ToString());
		return Result;
	}
	
	// 计算可堆叠的数量
	int32 AvailableSpace = ItemDef.MaxStackSize - TargetItem->Quantity;
	int32 StackQuantity = FMath::Min(Quantity, AvailableSpace);
	
	if (StackQuantity <= 0)
	{
		Result.ErrorMessage = TEXT("目标物品堆叠已满");
		Result.Result = EMoveItemResult::StackLimitReached;
		UE_LOG(LogGaia, Warning, TEXT("[StackItems] 目标物品堆叠已满 - 当前数量: %d, 最大堆叠: %d"), 
			TargetItem->Quantity, ItemDef.MaxStackSize);
		return Result;
	}
	
	// 更新目标物品数量
	int32 OldTargetQuantity = TargetItem->Quantity;
	TargetItem->Quantity += StackQuantity;
	
	UE_LOG(LogGaia, Verbose, TEXT("[StackItems] 更新目标物品数量: %d -> %d"), 
		OldTargetQuantity, TargetItem->Quantity);
	
	// 更新源物品数量
	int32 OldSourceQuantity = SourceItem->Quantity;
	SourceItem->Quantity -= StackQuantity;
	
	UE_LOG(LogGaia, Verbose, TEXT("[StackItems] 更新源物品数量: %d -> %d"), 
		OldSourceQuantity, SourceItem->Quantity);
	
	// 如果源物品数量为0，删除源物品
	if (SourceItem->Quantity <= 0)
	{
		UE_LOG(LogGaia, Log, TEXT("[StackItems] 源物品数量为0，准备删除: UID=%s, 容器=%s, 槽位=%d"), 
			*SourceItem->InstanceUID.ToString(), 
			*SourceItem->CurrentContainerUID.ToString(), 
			SourceItem->CurrentSlotID);
		
		DestroyItem(SourceItem->InstanceUID);
		
		UE_LOG(LogGaia, Log, TEXT("[StackItems] 源物品已删除: UID=%s"), *SourceItem->InstanceUID.ToString());
	}
	else
	{
		// 标记源容器需要重新计算
		if (SourceItem->IsInContainer())
		{
			if (FGaiaContainerInstance* SourceContainer = Containers.Find(SourceItem->CurrentContainerUID))
			{
				SourceContainer->MarkDirty();
			}
		}
	}
	
	// 标记目标容器需要重新计算
	if (TargetItem->IsInContainer())
	{
		if (FGaiaContainerInstance* TargetContainer = Containers.Find(TargetItem->CurrentContainerUID))
		{
			TargetContainer->MarkDirty();
		}
	}
	
	// 设置结果
	Result.Result = (StackQuantity == Quantity) ? EMoveItemResult::Success : EMoveItemResult::PartialSuccess;
	Result.MovedQuantity = StackQuantity;
	Result.RemainingQuantity = Quantity - StackQuantity;
	Result.TargetContainerUID = TargetItem->CurrentContainerUID;
	
	UE_LOG(LogGaia, Log, TEXT("[StackItems] 堆叠完成: 移动了 %d 个物品（请求 %d）"), 
		StackQuantity, Quantity);
	
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::MoveToItemContainer(FGaiaItemInstance* Item, FGaiaItemInstance* ContainerItem, int32 Quantity)
{
	// 参数已在调用者处验证，此处使用断言确保契约
	check(Item);
	check(ContainerItem);
	check(ContainerItem->HasContainer());
	check(Quantity > 0 && Quantity <= Item->Quantity);
	
	FMoveItemResult Result;
	
	// 获取容器物品的容器
	FGaiaContainerInstance* TargetContainer = Containers.Find(ContainerItem->OwnedContainerUID);
	if (!TargetContainer)
	{
		Result.ErrorMessage = TEXT("无法找到容器物品的容器");
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Error, TEXT("[MoveToItemContainer] 容器物品的容器不存在: %s"), *ContainerItem->OwnedContainerUID.ToString());
		return Result;
	}
	
	// 找到空槽位
	const int32 EmptySlotID = TargetContainer->FindEmptySlotID();
	if (EmptySlotID == INDEX_NONE)
	{
		Result.ErrorMessage = TEXT("容器已满");
		Result.Result = EMoveItemResult::ContainerFull;
		UE_LOG(LogGaia, Warning, TEXT("[MoveToItemContainer] 容器已满"));
		return Result;
	}
	
	// 先从源容器移除槽位引用
	if (Item->IsInContainer())
	{
		if (FGaiaContainerInstance* SourceContainer = Containers.Find(Item->CurrentContainerUID))
		{
			int32 SlotIndex = SourceContainer->GetSlotIndexByID(Item->CurrentSlotID);
			if (SlotIndex != INDEX_NONE)
			{
				SourceContainer->Slots[SlotIndex].ItemInstanceUID = FGuid();
				SourceContainer->MarkDirty();
				UE_LOG(LogGaia, Verbose, TEXT("[MoveToItemContainer] 清空源槽位引用: 容器=%s, 槽位=%d"),
					*Item->CurrentContainerUID.ToString(), Item->CurrentSlotID);
			}
		}
	}
	
	// 添加物品到目标容器
	if (AddItemToContainer(Item, TargetContainer, EmptySlotID))
	{
		Result.Result = EMoveItemResult::Success;
		Result.MovedQuantity = Quantity;
		Result.RemainingQuantity = 0;
		Result.bMovedToContainer = true;
		Result.TargetContainerUID = ContainerItem->OwnedContainerUID;
		UE_LOG(LogGaia, Log, TEXT("[MoveToItemContainer] 成功将物品放入容器物品的容器 (数量: %d)"), Quantity);
		return Result;
	}
	
	Result.ErrorMessage = TEXT("添加物品到容器失败");
	Result.Result = EMoveItemResult::Failed;
	UE_LOG(LogGaia, Error, TEXT("[MoveToItemContainer] 添加物品到容器失败"));
	return Result;
}

bool UGaiaInventorySubsystem::SwapItems(FGaiaItemInstance* Item1, FGaiaItemInstance* Item2)
{
	// 参数已在调用者处验证，此处使用断言确保契约
	check(Item1);
	check(Item2);
	check(Item1->IsInContainer());
	check(Item2->IsInContainer());
	
	// 获取原始位置
	FGuid Container1UID = Item1->CurrentContainerUID;
	int32 Slot1ID = Item1->CurrentSlotID;
	FGuid Container2UID = Item2->CurrentContainerUID;
	int32 Slot2ID = Item2->CurrentSlotID;
	
	UE_LOG(LogGaia, Verbose, TEXT("[SwapItems] 交换物品位置: Item1在容器%s槽位%d <-> Item2在容器%s槽位%d"), 
		*Container1UID.ToString(), Slot1ID,
		*Container2UID.ToString(), Slot2ID);
	
	// 获取容器
	FGaiaContainerInstance* Container1 = Containers.Find(Container1UID);
	FGaiaContainerInstance* Container2 = Containers.Find(Container2UID);
	
	if (!Container1 || !Container2)
	{
		UE_LOG(LogGaia, Error, TEXT("[SwapItems] 无法找到容器"));
		return false;
	}
	
	// 交换物品的位置信息
	Item1->CurrentContainerUID = Container2UID;
	Item1->CurrentSlotID = Slot2ID;
	Item2->CurrentContainerUID = Container1UID;
	Item2->CurrentSlotID = Slot1ID;
	
	// 交换槽位引用
	int32 SlotIndex1 = Container1->GetSlotIndexByID(Slot1ID);
	int32 SlotIndex2 = Container2->GetSlotIndexByID(Slot2ID);
	
	if (SlotIndex1 != INDEX_NONE)
	{
		Container1->Slots[SlotIndex1].ItemInstanceUID = Item2->InstanceUID;
		UE_LOG(LogGaia, Verbose, TEXT("[SwapItems] Container1槽位%d更新: %s -> %s"), 
			Slot1ID, *Item1->InstanceUID.ToString(), *Item2->InstanceUID.ToString());
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[SwapItems] 无法找到Container1的槽位索引，SlotID: %d"), Slot1ID);
		return false;
	}
	
	if (SlotIndex2 != INDEX_NONE)
	{
		Container2->Slots[SlotIndex2].ItemInstanceUID = Item1->InstanceUID;
		UE_LOG(LogGaia, Verbose, TEXT("[SwapItems] Container2槽位%d更新: %s -> %s"), 
			Slot2ID, *Item2->InstanceUID.ToString(), *Item1->InstanceUID.ToString());
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[SwapItems] 无法找到Container2的槽位索引，SlotID: %d"), Slot2ID);
		return false;
	}
	
	// 更新嵌套容器的父容器（如果有）
	if (Item1->HasContainer())
	{
		if (FGaiaContainerInstance* Item1Container = Containers.Find(Item1->OwnedContainerUID))
		{
			Item1Container->ParentContainerUID = Container2UID;
		}
	}
	
	if (Item2->HasContainer())
	{
		if (FGaiaContainerInstance* Item2Container = Containers.Find(Item2->OwnedContainerUID))
		{
			Item2Container->ParentContainerUID = Container1UID;
		}
	}
	
	// 标记需要重新计算
	Container1->MarkDirty();
	Container2->MarkDirty();
	
	UE_LOG(LogGaia, Verbose, TEXT("[SwapItems] 交换成功: %s <-> %s"), 
		*Item1->InstanceUID.ToString(), *Item2->InstanceUID.ToString());
	
	return true;
}

FMoveItemResult UGaiaInventorySubsystem::CanSwapItems(FGaiaItemInstance* Item1, FGaiaItemInstance* Item2) const
{
	check(Item1);
	check(Item2);
	
	FMoveItemResult Result;
	
	// 检查是否都在容器中
	if (!Item1->IsInContainer() || !Item2->IsInContainer())
	{
		Result.Result = EMoveItemResult::Failed;
		Result.ErrorMessage = FString::Printf(TEXT("物品不在容器中 (Item1在容器: %s, Item2在容器: %s)"),
			Item1->IsInContainer() ? TEXT("是") : TEXT("否"),
			Item2->IsInContainer() ? TEXT("是") : TEXT("否"));
		return Result;
	}
	
	// 如果在同一个容器中，直接允许交换（同容器内交换不会造成循环）
	if (Item1->CurrentContainerUID == Item2->CurrentContainerUID)
	{
		Result.Result = EMoveItemResult::Success;
		return Result;
	}
	
	// 跨容器交换，需要检查是否可以添加到对方的容器
	const FGaiaContainerInstance* Container1 = Containers.Find(Item1->CurrentContainerUID);
	const FGaiaContainerInstance* Container2 = Containers.Find(Item2->CurrentContainerUID);
	
	if (!Container1 || !Container2)
	{
		Result.Result = EMoveItemResult::InvalidTarget;
		Result.ErrorMessage = FString::Printf(TEXT("无法找到容器 (Container1: %s, Container2: %s)"),
			Container1 ? TEXT("存在") : TEXT("不存在"),
			Container2 ? TEXT("存在") : TEXT("不存在"));
		return Result;
	}
	
	// 检查 Item1 是否可以放入 Container2
	FAddItemResult CanAdd1To2 = CanAddItemToContainer(Item1, const_cast<FGaiaContainerInstance*>(Container2));
	if (!CanAdd1To2.IsSuccess())
	{
		Result.Result = EMoveItemResult::Failed;
		Result.ErrorMessage = FString::Printf(TEXT("Item1无法放入Container2: %s"), *CanAdd1To2.ErrorMessage);
		return Result;
	}
	
	// 检查 Item2 是否可以放入 Container1
	FAddItemResult CanAdd2To1 = CanAddItemToContainer(Item2, const_cast<FGaiaContainerInstance*>(Container1));
	if (!CanAdd2To1.IsSuccess())
	{
		Result.Result = EMoveItemResult::Failed;
		Result.ErrorMessage = FString::Printf(TEXT("Item2无法放入Container1: %s"), *CanAdd2To1.ErrorMessage);
		return Result;
	}
	
	Result.Result = EMoveItemResult::Success;
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::MoveToEmptySlot(FGaiaItemInstance* Item, FGaiaContainerInstance* TargetContainer, int32 TargetSlotID, int32 Quantity)
{
	// 参数已在调用者处验证，此处使用断言确保契约
	check(Item);
	check(TargetContainer);
	check(TargetSlotID >= 0);
	check(Quantity > 0 && Quantity <= Item->Quantity);
	
	FMoveItemResult Result;
	
	// 找到目标槽位
	int32 TargetSlotIndex = TargetContainer->GetSlotIndexByID(TargetSlotID);
	if (TargetSlotIndex == INDEX_NONE)
	{
		Result.ErrorMessage = FString::Printf(TEXT("目标槽位无效 (SlotID: %d)"), TargetSlotID);
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Error, TEXT("[MoveToEmptySlot] %s"), *Result.ErrorMessage);
		return Result;
	}
	
	// 如果是部分移动，创建新物品实例
	bool bIsPartialMove = (Quantity < Item->Quantity);
	
	if (bIsPartialMove)
	{
		// 部分移动：创建新物品
		FGaiaItemInstance NewItem = *Item;
		NewItem.InstanceUID = FGuid::NewGuid();
		NewItem.Quantity = Quantity;
		NewItem.CurrentContainerUID = TargetContainer->ContainerUID;
		NewItem.CurrentSlotID = TargetSlotID;
		
		// 添加新物品到AllItems
		AllItems.Add(NewItem.InstanceUID, NewItem);
		
		// 更新目标槽位引用
		TargetContainer->Slots[TargetSlotIndex].ItemInstanceUID = NewItem.InstanceUID;
		
		// 减少源物品数量
		Item->Quantity -= Quantity;
		
		// 标记容器需要重新计算
		if (Item->IsInContainer())
		{
			if (FGaiaContainerInstance* SourceContainer = Containers.Find(Item->CurrentContainerUID))
			{
				SourceContainer->MarkDirty();
			}
		}
		TargetContainer->MarkDirty();
		
		UE_LOG(LogGaia, Verbose, TEXT("[MoveToEmptySlot] 部分移动: 原UID=%s (剩余%d), 新UID=%s (移动%d)"), 
			*Item->InstanceUID.ToString(), Item->Quantity, *NewItem.InstanceUID.ToString(), Quantity);
	}
	else
	{
		// 完全移动：直接更新物品位置
		// 先从源容器移除
		if (Item->IsInContainer())
		{
			if (FGaiaContainerInstance* SourceContainer = Containers.Find(Item->CurrentContainerUID))
			{
				int32 SourceSlotIndex = SourceContainer->GetSlotIndexByID(Item->CurrentSlotID);
				if (SourceSlotIndex != INDEX_NONE)
				{
					SourceContainer->Slots[SourceSlotIndex].ItemInstanceUID = FGuid();
				}
				SourceContainer->MarkDirty();
			}
		}
		
		// 更新物品位置
		Item->CurrentContainerUID = TargetContainer->ContainerUID;
		Item->CurrentSlotID = TargetSlotID;
		
		// 更新目标槽位引用
		TargetContainer->Slots[TargetSlotIndex].ItemInstanceUID = Item->InstanceUID;
		TargetContainer->MarkDirty();
		
		UE_LOG(LogGaia, Verbose, TEXT("[MoveToEmptySlot] 完全移动: UID=%s, 数量=%d"), 
			*Item->InstanceUID.ToString(), Quantity);
	}
	
	Result.Result = EMoveItemResult::Success;
	Result.MovedQuantity = Quantity;
	Result.RemainingQuantity = bIsPartialMove ? Item->Quantity : 0;
	Result.TargetContainerUID = TargetContainer->ContainerUID;
	
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::MoveItemWithinContainer(FGaiaItemInstance* Item, int32 TargetSlotID, int32 Quantity)
{
	// 参数已在调用者处验证，此处使用断言确保契约
	check(Item);
	check(Item->IsInContainer());
	check(TargetSlotID >= 0);
	check(Quantity > 0 && Quantity <= Item->Quantity);
	
	FMoveItemResult Result;
	
	// 获取容器
	FGaiaContainerInstance* Container = Containers.Find(Item->CurrentContainerUID);
	if (!Container)
	{
		Result.ErrorMessage = TEXT("无法找到容器");
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("[MoveItemWithinContainer] 无法找到容器: %s"), *Item->CurrentContainerUID.ToString());
		return Result;
	}
	
	// 获取目标槽位
	int32 TargetSlotIndex = Container->GetSlotIndexByID(TargetSlotID);
	if (TargetSlotIndex == INDEX_NONE)
	{
		Result.ErrorMessage = FString::Printf(TEXT("目标槽位无效: %d"), TargetSlotID);
		Result.Result = EMoveItemResult::InvalidTarget;
		UE_LOG(LogGaia, Error, TEXT("[MoveItemWithinContainer] %s"), *Result.ErrorMessage);
		return Result;
	}
	
	FGaiaSlotInfo& TargetSlot = Container->Slots[TargetSlotIndex];
	
	if (TargetSlot.IsEmpty())
	{
		// 目标槽位为空，调用 MoveToEmptySlot 处理
		UE_LOG(LogGaia, Verbose, TEXT("[MoveItemWithinContainer] 目标槽位为空，调用 MoveToEmptySlot"));
		return MoveToEmptySlot(Item, Container, TargetSlotID, Quantity);
	}
	
	// 目标槽位有物品，需要处理
	if (FGaiaItemInstance* TargetItem = AllItems.Find(TargetSlot.ItemInstanceUID))
	{
		// 检查是否为相同类型（尝试堆叠）
		if (Item->ItemDefinitionID == TargetItem->ItemDefinitionID)
		{
			// TODO: 需要添加堆叠检查逻辑
			return StackItems(Item, TargetItem, Quantity);
		}
		
		// 不同类型，尝试交换位置
		if (!SwapItems(Item, TargetItem))
		{
			Result.ErrorMessage = TEXT("交换物品失败");
			Result.Result = EMoveItemResult::Failed;
			return Result;
		}
		
		Result.Result = EMoveItemResult::SwapPerformed;
		Result.bWasSwapped = true;
		Result.SwappedItemUID = TargetItem->InstanceUID;
		UE_LOG(LogGaia, Log, TEXT("[MoveItemWithinContainer] 交换成功: %s <-> %s"), 
			*Item->InstanceUID.ToString(), *TargetItem->InstanceUID.ToString());
		return Result;
	}
	
	// 无法找到目标物品（数据不一致）
	Result.ErrorMessage = TEXT("目标槽位数据异常：物品不存在");
	Result.Result = EMoveItemResult::Failed;
	UE_LOG(LogGaia, Error, TEXT("[MoveItemWithinContainer] 无法找到目标物品: %s"),
		*TargetSlot.ItemInstanceUID.ToString());
	
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::ProcessTargetSlotWithItem(FGaiaItemInstance* SourceItem, FGaiaItemInstance* TargetItem, FGaiaContainerInstance* TargetContainer, int32 TargetSlotID, int32 Quantity)
{
	// 参数已在调用者处验证，此处使用断言确保契约
	check(SourceItem);
	check(TargetItem);
	check(TargetContainer);
	check(TargetSlotID >= 0);
	check(Quantity > 0 && Quantity <= SourceItem->Quantity);
	
	FMoveItemResult Result;
	
	// 检查是否为相同类型（尝试堆叠）
	if (SourceItem->ItemDefinitionID == TargetItem->ItemDefinitionID)
	{
		// TODO: 需要添加堆叠检查逻辑
		UE_LOG(LogGaia, Verbose, TEXT("[ProcessTargetSlotWithItem] 相同类型，尝试堆叠"));
		return StackItems(SourceItem, TargetItem, Quantity);
	}
	
	// 检查目标物品是否有容器（尝试放入容器）
	if (TargetItem->HasContainer())
	{
		// TODO: 需要添加容器检查逻辑
		UE_LOG(LogGaia, Verbose, TEXT("[ProcessTargetSlotWithItem] 目标物品有容器，尝试放入"));
		FMoveItemResult ContainerResult = MoveToItemContainer(SourceItem, TargetItem, Quantity);
		if (ContainerResult.Result == EMoveItemResult::Success)
		{
			return ContainerResult;
		}
		// 如果放入容器失败，不交换位置，直接返回失败
		Result.ErrorMessage = TEXT("无法放入目标物品的容器");
		Result.Result = EMoveItemResult::ContainerRejected;
		UE_LOG(LogGaia, Warning, TEXT("[ProcessTargetSlotWithItem] 放入容器失败"));
		return Result;
	}
	
	// 不同类型且目标无容器，尝试交换位置
	UE_LOG(LogGaia, Verbose, TEXT("[ProcessTargetSlotWithItem] 尝试交换位置"));
	if (!SwapItems(SourceItem, TargetItem))
	{
		Result.ErrorMessage = TEXT("交换物品失败");
		Result.Result = EMoveItemResult::Failed;
		UE_LOG(LogGaia, Error, TEXT("[ProcessTargetSlotWithItem] 交换失败"));
		return Result;
	}
	
	Result.Result = EMoveItemResult::SwapPerformed;
	Result.bWasSwapped = true;
	Result.SwappedItemUID = TargetItem->InstanceUID;
	UE_LOG(LogGaia, Log, TEXT("[ProcessTargetSlotWithItem] 交换成功: %s <-> %s"), 
		*SourceItem->InstanceUID.ToString(), *TargetItem->InstanceUID.ToString());
	return Result;
}

FMoveItemResult UGaiaInventorySubsystem::MoveItemAutoSlot(FGaiaItemInstance* Item, FGaiaContainerInstance* TargetContainer, int32 Quantity)
{
	// 参数已在调用者处验证，此处使用断言确保契约
	check(Item);
	check(TargetContainer);
	check(Quantity > 0 && Quantity <= Item->Quantity);
	
	FMoveItemResult Result;
	
	// 1. 优先查找空槽位
	int32 EmptySlotID = TargetContainer->FindEmptySlotID();
	if (EmptySlotID != INDEX_NONE)
	{
		UE_LOG(LogGaia, Verbose, TEXT("[MoveItemAutoSlot] 找到空槽位: %d"), EmptySlotID);
		return MoveToEmptySlot(Item, TargetContainer, EmptySlotID, Quantity);
	}
	
	UE_LOG(LogGaia, Verbose, TEXT("[MoveItemAutoSlot] 无空槽位，尝试堆叠或放入嵌套容器"));
	
	// 2. 没有空槽位，遍历所有槽位尝试堆叠或放入嵌套容器
	for (const FGaiaSlotInfo& Slot : TargetContainer->Slots)
	{
		if (Slot.IsEmpty())
			continue;
			
		FGaiaItemInstance* TargetItem = AllItems.Find(Slot.ItemInstanceUID);
		if (!TargetItem)
			continue;
		
		// 2.1 尝试堆叠（相同类型）
		if (Item->ItemDefinitionID == TargetItem->ItemDefinitionID)
		{
			// TODO: 需要添加堆叠检查逻辑
			UE_LOG(LogGaia, Verbose, TEXT("[MoveItemAutoSlot] 尝试堆叠到物品: %s"), *TargetItem->InstanceUID.ToString());
			FMoveItemResult StackResult = StackItems(Item, TargetItem, Quantity);
			if (StackResult.IsSuccess())
			{
				UE_LOG(LogGaia, Log, TEXT("[MoveItemAutoSlot] 堆叠成功"));
				return StackResult;
			}
		}
		
		// 2.2 尝试放入嵌套容器
		if (TargetItem->HasContainer())
		{
			// TODO: 需要添加容器检查逻辑
			UE_LOG(LogGaia, Verbose, TEXT("[MoveItemAutoSlot] 尝试放入嵌套容器: %s"), *TargetItem->InstanceUID.ToString());
			FMoveItemResult ContainerResult = MoveToItemContainer(Item, TargetItem, Quantity);
			if (ContainerResult.Result == EMoveItemResult::Success)
			{
				UE_LOG(LogGaia, Log, TEXT("[MoveItemAutoSlot] 成功放入嵌套容器"));
				return ContainerResult;
			}
		}
	}
	
	// 3. 无法找到合适的位置
	Result.ErrorMessage = TEXT("无法找到合适的位置移动物品");
	Result.Result = EMoveItemResult::ContainerFull;
	UE_LOG(LogGaia, Warning, TEXT("[MoveItemAutoSlot] 容器已满，无法移动"));
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

void UGaiaInventorySubsystem::GetAllContainers(TArray<FGaiaContainerInstance>& OutContainers) const
{
	OutContainers.Empty();
	Containers.GenerateValueArray(OutContainers);
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

void UGaiaInventorySubsystem::BroadcastContainerUpdate(const FGuid& ContainerUID)
{
	// 简化版本：通知所有玩家刷新
	// TODO: 后续可以优化为只通知相关玩家
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (!PC) continue;
		
		// 查找玩家的 RPC 组件
		UGaiaInventoryRPCComponent* RPCComp = PC->FindComponentByClass<UGaiaInventoryRPCComponent>();
		if (RPCComp)
		{
			// 通知客户端刷新库存数据
			RPCComp->ServerRequestRefreshInventory_Implementation();
		}
	}
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

