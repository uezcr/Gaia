// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaInventoryRPCComponent.h"
#include "GaiaInventorySubsystem.h"
#include "GaiaLogChannels.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UGaiaInventoryRPCComponent::UGaiaInventoryRPCComponent()
{
	// 启用网络复制
	SetIsReplicatedByDefault(true);
	
	// 默认不Tick
	PrimaryComponentTick.bCanEverTick = false;
}

void UGaiaInventoryRPCComponent::BeginPlay()
{
	Super::BeginPlay();

	// 缓存Subsystem引用
	CachedSubsystem = GetInventorySubsystem();

	// 客户端：请求初始数据
	if (GetOwnerRole() != ROLE_Authority)
	{
		RequestRefreshInventory();
	}
}

void UGaiaInventoryRPCComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 复制拥有的容器列表
	DOREPLIFETIME(UGaiaInventoryRPCComponent, OwnedContainerUIDs);
	
	// 复制打开的世界容器列表
	DOREPLIFETIME(UGaiaInventoryRPCComponent, OpenWorldContainerUIDs);
}

// ========================================
// 客户端请求接口
// ========================================

void UGaiaInventoryRPCComponent::RequestMoveItem(
	const FGuid& ItemUID,
	const FGuid& TargetContainerUID,
	int32 TargetSlotID,
	int32 Quantity)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		// 服务器直接执行
		ServerMoveItem_Implementation(ItemUID, TargetContainerUID, TargetSlotID, Quantity);
	}
	else
	{
		// 客户端发送RPC
		ServerMoveItem(ItemUID, TargetContainerUID, TargetSlotID, Quantity);
	}
}

void UGaiaInventoryRPCComponent::RequestAddItem(
	const FGuid& ItemUID,
	const FGuid& ContainerUID)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ServerAddItem_Implementation(ItemUID, ContainerUID);
	}
	else
	{
		ServerAddItem(ItemUID, ContainerUID);
	}
}

void UGaiaInventoryRPCComponent::RequestRemoveItem(const FGuid& ItemUID)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ServerRemoveItem_Implementation(ItemUID);
	}
	else
	{
		ServerRemoveItem(ItemUID);
	}
}

void UGaiaInventoryRPCComponent::RequestDestroyItem(const FGuid& ItemUID)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ServerDestroyItem_Implementation(ItemUID);
	}
	else
	{
		ServerDestroyItem(ItemUID);
	}
}

void UGaiaInventoryRPCComponent::RequestOpenWorldContainer(const FGuid& ContainerUID)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ServerOpenWorldContainer_Implementation(ContainerUID);
	}
	else
	{
		ServerOpenWorldContainer(ContainerUID);
	}
}

void UGaiaInventoryRPCComponent::RequestCloseWorldContainer(const FGuid& ContainerUID)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ServerCloseWorldContainer_Implementation(ContainerUID);
	}
	else
	{
		ServerCloseWorldContainer(ContainerUID);
	}
}

void UGaiaInventoryRPCComponent::RequestRefreshInventory()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ServerRequestRefreshInventory_Implementation();
	}
	else
	{
		ServerRequestRefreshInventory();
	}
}

// ========================================
// 服务器RPC实现
// ========================================

void UGaiaInventoryRPCComponent::ServerMoveItem_Implementation(
	const FGuid& ItemUID,
	const FGuid& TargetContainerUID,
	int32 TargetSlotID,
	int32 Quantity)
{
	UGaiaInventorySubsystem* InventorySystem = GetInventorySubsystem();
	if (!InventorySystem)
	{
		ClientOperationFailed(1, TEXT("库存系统不可用"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	// 执行移动
	FMoveItemResult Result = InventorySystem->TryMoveItem(ItemUID, TargetContainerUID, TargetSlotID, Quantity);

	if (Result.IsSuccess())
	{
		UE_LOG(LogGaia, Log, TEXT("[网络] 玩家 %s 成功移动物品: %s -> 容器 %s 槽位 %d"),
			*PC->GetName(), *ItemUID.ToString(), *TargetContainerUID.ToString(), TargetSlotID);

		// 广播给所有访问目标容器的玩家
		InventorySystem->BroadcastContainerUpdate(TargetContainerUID);
		
		// 如果源容器不同，也广播源容器更新
		FGaiaItemInstance Item;
		if (InventorySystem->FindItemByUID(ItemUID, Item) && Item.CurrentContainerUID.IsValid() && Item.CurrentContainerUID != TargetContainerUID)
		{
			InventorySystem->BroadcastContainerUpdate(Item.CurrentContainerUID);
		}
	}
	else
	{
		UE_LOG(LogGaia, Warning, TEXT("[网络] 玩家 %s 移动物品失败: %s"),
			*PC->GetName(), *Result.ErrorMessage);

		ClientOperationFailed(3, Result.ErrorMessage);
	}
}

bool UGaiaInventoryRPCComponent::ServerMoveItem_Validate(
	const FGuid& ItemUID,
	const FGuid& TargetContainerUID,
	int32 TargetSlotID,
	int32 Quantity)
{
	// 基本验证，防止恶意数据
	return TargetSlotID >= -1 && TargetSlotID < 1000 && Quantity >= 0 && Quantity <= 9999;
}

void UGaiaInventoryRPCComponent::ServerAddItem_Implementation(
	const FGuid& ItemUID,
	const FGuid& ContainerUID)
{
	UGaiaInventorySubsystem* InventorySystem = GetInventorySubsystem();
	if (!InventorySystem)
	{
		ClientOperationFailed(1, TEXT("库存系统不可用"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	// 查找物品
	FGaiaItemInstance Item;
	if (!InventorySystem->FindItemByUID(ItemUID, Item))
	{
		ClientOperationFailed(4, TEXT("物品不存在"));
		return;
	}

	// 添加到容器
	FAddItemResult AddResult = InventorySystem->TryAddItemToContainer(Item.InstanceUID, ContainerUID);
	if (AddResult.IsSuccess())
	{
		UE_LOG(LogGaia, Log, TEXT("[网络] 添加物品成功: %s -> 容器 %s"), 
			*ItemUID.ToString(), *ContainerUID.ToString());
		
		// 广播容器更新
		InventorySystem->BroadcastContainerUpdate(ContainerUID);
	}
	else
	{
		ClientOperationFailed(5, TEXT("添加物品失败"));
	}
}

bool UGaiaInventoryRPCComponent::ServerAddItem_Validate(
	const FGuid& ItemUID,
	const FGuid& ContainerUID)
{
	return ItemUID.IsValid() && ContainerUID.IsValid();
}

void UGaiaInventoryRPCComponent::ServerRemoveItem_Implementation(const FGuid& ItemUID)
{
	UGaiaInventorySubsystem* InventorySystem = GetInventorySubsystem();
	if (!InventorySystem)
	{
		ClientOperationFailed(1, TEXT("库存系统不可用"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	// 先获取物品所在容器
	FGaiaItemInstance Item;
	FGuid SourceContainerUID;
	if (InventorySystem->FindItemByUID(ItemUID, Item) && Item.IsInContainer())
	{
		SourceContainerUID = Item.CurrentContainerUID;
	}

	if (InventorySystem->RemoveItemFromContainer(ItemUID))
	{
		UE_LOG(LogGaia, Log, TEXT("[网络] 移除物品成功: %s"), *ItemUID.ToString());
		
		// 广播源容器更新
		if (SourceContainerUID.IsValid())
		{
			InventorySystem->BroadcastContainerUpdate(SourceContainerUID);
		}
	}
	else
	{
		ClientOperationFailed(6, TEXT("移除物品失败"));
	}
}

bool UGaiaInventoryRPCComponent::ServerRemoveItem_Validate(const FGuid& ItemUID)
{
	return ItemUID.IsValid();
}

void UGaiaInventoryRPCComponent::ServerDestroyItem_Implementation(const FGuid& ItemUID)
{
	UGaiaInventorySubsystem* InventorySystem = GetInventorySubsystem();
	if (!InventorySystem)
	{
		ClientOperationFailed(1, TEXT("库存系统不可用"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	// 先获取物品所在容器
	FGaiaItemInstance Item;
	FGuid SourceContainerUID;
	if (InventorySystem->FindItemByUID(ItemUID, Item) && Item.IsInContainer())
	{
		SourceContainerUID = Item.CurrentContainerUID;
	}

	if (InventorySystem->DestroyItem(ItemUID))
	{
		UE_LOG(LogGaia, Log, TEXT("[网络] 销毁物品成功: %s"), *ItemUID.ToString());
		
		// 广播源容器更新
		if (SourceContainerUID.IsValid())
		{
			InventorySystem->BroadcastContainerUpdate(SourceContainerUID);
		}
	}
	else
	{
		ClientOperationFailed(7, TEXT("销毁物品失败"));
	}
}

bool UGaiaInventoryRPCComponent::ServerDestroyItem_Validate(const FGuid& ItemUID)
{
	return ItemUID.IsValid();
}

void UGaiaInventoryRPCComponent::ServerOpenWorldContainer_Implementation(const FGuid& ContainerUID)
{
	UGaiaInventorySubsystem* InventorySystem = GetInventorySubsystem();
	if (!InventorySystem)
	{
		ClientOperationFailed(1, TEXT("库存系统不可用"));
		return;
	}

	// 检查容器是否存在
	FGaiaContainerInstance Container;
	if (!InventorySystem->FindContainerByUID(ContainerUID, Container))
	{
		ClientOperationFailed(8, TEXT("容器不存在"));
		return;
	}

	// 添加到打开的世界容器列表
	if (!OpenWorldContainerUIDs.Contains(ContainerUID))
	{
		OpenWorldContainerUIDs.Add(ContainerUID);
	}

	// 刷新库存数据（包含新打开的容器）
	ServerRequestRefreshInventory_Implementation();

	// 触发打开事件
	OnContainerOpened.Broadcast(ContainerUID);
}

bool UGaiaInventoryRPCComponent::ServerOpenWorldContainer_Validate(const FGuid& ContainerUID)
{
	return ContainerUID.IsValid();
}

void UGaiaInventoryRPCComponent::ServerCloseWorldContainer_Implementation(const FGuid& ContainerUID)
{
	// 从打开的世界容器列表移除
	OpenWorldContainerUIDs.Remove(ContainerUID);

	// 触发关闭事件
	OnContainerClosed.Broadcast(ContainerUID);
}

bool UGaiaInventoryRPCComponent::ServerCloseWorldContainer_Validate(const FGuid& ContainerUID)
{
	return ContainerUID.IsValid();
}

void UGaiaInventoryRPCComponent::ServerRequestRefreshInventory_Implementation()
{
	UE_LOG(LogGaia, Log, TEXT("[RPC组件] ServerRequestRefreshInventory 被调用"));
	
	UGaiaInventorySubsystem* InventorySystem = GetInventorySubsystem();
	if (!InventorySystem)
	{
		UE_LOG(LogGaia, Error, TEXT("[RPC组件] 无法获取InventorySubsystem"));
		return;
	}

	// 收集所有需要同步的容器（包括嵌套容器）
	TSet<FGuid> ContainerUIDsToSync;
	TSet<FGuid> ProcessedContainers; // 防止循环引用导致的无限递归
	
	// 1. 添加玩家拥有的容器
	for (const FGuid& UID : OwnedContainerUIDs)
	{
		if (UID.IsValid())
		{
			ContainerUIDsToSync.Add(UID);
		}
	}

	// 2. 添加打开的世界容器
	for (const FGuid& UID : OpenWorldContainerUIDs)
	{
		if (UID.IsValid())
		{
			ContainerUIDsToSync.Add(UID);
		}
	}

	// 3. 递归收集嵌套容器
	TArray<FGuid> ContainersToProcess = ContainerUIDsToSync.Array();
	while (ContainersToProcess.Num() > 0)
	{
		FGuid CurrentContainerUID = ContainersToProcess.Pop();
		
		// 避免重复处理
		if (ProcessedContainers.Contains(CurrentContainerUID))
		{
			continue;
		}
		ProcessedContainers.Add(CurrentContainerUID);
		
		// 获取容器中的所有物品
		TArray<FGaiaItemInstance> Items = InventorySystem->GetItemsInContainer(CurrentContainerUID);
		for (const FGaiaItemInstance& Item : Items)
		{
			// 如果物品有嵌套容器，添加到处理队列
			if (Item.HasContainer() && !ProcessedContainers.Contains(Item.OwnedContainerUID))
			{
				ContainerUIDsToSync.Add(Item.OwnedContainerUID);
				ContainersToProcess.Add(Item.OwnedContainerUID);
			}
		}
	}

	UE_LOG(LogGaia, Log, TEXT("[RPC组件] 需要同步的容器总数: %d (包括嵌套容器)"), ContainerUIDsToSync.Num());

	// 收集数据
	TArray<FGaiaItemInstance> PlayerItems;
	TArray<FGaiaContainerInstance> PlayerContainers;

	for (const FGuid& ContainerUID : ContainerUIDsToSync)
	{
		FGaiaContainerInstance Container;
		if (InventorySystem->FindContainerByUID(ContainerUID, Container))
		{
			PlayerContainers.Add(Container);

			// 收集容器中的物品
			TArray<FGaiaItemInstance> Items = InventorySystem->GetItemsInContainer(ContainerUID);
			PlayerItems.Append(Items);
		}
	}

	// 发送给客户端
	UE_LOG(LogGaia, Log, TEXT("[RPC组件] 发送数据到客户端: %d 个物品, %d 个容器"),
		PlayerItems.Num(), PlayerContainers.Num());
	
	ClientReceiveInventoryData(PlayerItems, PlayerContainers);
}

// ========================================
// 客户端RPC实现
// ========================================

void UGaiaInventoryRPCComponent::ClientReceiveInventoryData_Implementation(
	const TArray<FGaiaItemInstance>& Items,
	const TArray<FGaiaContainerInstance>& Containers)
{
	UE_LOG(LogGaia, Warning, TEXT("[RPC组件] ⭐⭐⭐ ClientReceiveInventoryData 被调用: %d 个物品, %d 个容器"),
		Items.Num(), Containers.Num());

	// 更新本地缓存
	CachedItems.Empty();
	for (const FGaiaItemInstance& Item : Items)
	{
		CachedItems.Add(Item.InstanceUID, Item);
	}

	CachedContainers.Empty();
	for (const FGaiaContainerInstance& Container : Containers)
	{
		CachedContainers.Add(Container.ContainerUID, Container);
	}

	// 触发更新事件（UI可以监听此事件）
	UE_LOG(LogGaia, Warning, TEXT("[RPC组件] ⭐⭐⭐ 广播 OnInventoryUpdated 事件"));
	OnInventoryUpdated.Broadcast();
}

void UGaiaInventoryRPCComponent::ClientReceiveInventoryDelta_Implementation(
	const TArray<FGaiaItemInstance>& UpdatedItems,
	const TArray<FGuid>& RemovedItemUIDs,
	const TArray<FGaiaContainerInstance>& UpdatedContainers)
{
	UE_LOG(LogGaia, Log, TEXT("[网络] 客户端收到增量更新: %d 个更新物品, %d 个移除物品, %d 个更新容器"),
		UpdatedItems.Num(), RemovedItemUIDs.Num(), UpdatedContainers.Num());

	// 更新物品
	for (const FGaiaItemInstance& Item : UpdatedItems)
	{
		CachedItems.Add(Item.InstanceUID, Item);
	}

	// 移除物品
	for (const FGuid& ItemUID : RemovedItemUIDs)
	{
		CachedItems.Remove(ItemUID);
	}

	// 更新容器
	for (const FGaiaContainerInstance& Container : UpdatedContainers)
	{
		CachedContainers.Add(Container.ContainerUID, Container);
	}

	// 触发更新事件
	UE_LOG(LogGaia, Warning, TEXT("[RPC组件] ⭐ 广播 OnInventoryUpdated 事件，绑定数量: %d"),
		OnInventoryUpdated.IsBound() ? 1 : 0);
	OnInventoryUpdated.Broadcast();
}

void UGaiaInventoryRPCComponent::ClientOperationSuccess_Implementation(const FString& Message)
{
	UE_LOG(LogGaia, Log, TEXT("[网络] 操作成功: %s"), *Message);
	// UI 提示应在 UI 层通过监听 OnInventoryUpdated 事件处理
}

void UGaiaInventoryRPCComponent::ClientOperationFailed_Implementation(int32 ErrorCode, const FString& ErrorMessage)
{
	UE_LOG(LogGaia, Warning, TEXT("[网络] 操作失败 (错误码 %d): %s"), ErrorCode, *ErrorMessage);

	// 触发失败事件（UI可以显示错误提示）
	OnOperationFailed.Broadcast(ErrorCode, ErrorMessage);
}

// ========================================
// 客户端本地缓存访问
// ========================================

bool UGaiaInventoryRPCComponent::GetCachedItem(const FGuid& ItemUID, FGaiaItemInstance& OutItem) const
{
	if (const FGaiaItemInstance* Found = CachedItems.Find(ItemUID))
	{
		OutItem = *Found;
		return true;
	}
	return false;
}

bool UGaiaInventoryRPCComponent::GetCachedContainer(const FGuid& ContainerUID, FGaiaContainerInstance& OutContainer) const
{
	if (const FGaiaContainerInstance* Found = CachedContainers.Find(ContainerUID))
	{
		OutContainer = *Found;
		return true;
	}
	return false;
}

void UGaiaInventoryRPCComponent::AddOwnedContainerUID(const FGuid& ContainerUID)
{
	if (!ContainerUID.IsValid())
	{
		return;
	}

	if (!OwnedContainerUIDs.Contains(ContainerUID))
	{
		OwnedContainerUIDs.Add(ContainerUID);
		UE_LOG(LogGaia, Verbose, TEXT("[RPC组件] 添加拥有的容器UID: %s"), *ContainerUID.ToString());
	}
}

// ========================================
// 复制回调
// ========================================

void UGaiaInventoryRPCComponent::OnRep_OwnedContainers()
{
	UE_LOG(LogGaia, Verbose, TEXT("[网络] 拥有的容器列表已更新: %d 个"), OwnedContainerUIDs.Num());
	
	// 请求刷新数据
	RequestRefreshInventory();
}

void UGaiaInventoryRPCComponent::OnRep_OpenWorldContainers() const
{
	UE_LOG(LogGaia, Verbose, TEXT("[网络] 打开的世界容器列表已更新: %d 个"), OpenWorldContainerUIDs.Num());
}

// ========================================
// 内部辅助函数
// ========================================

UGaiaInventorySubsystem* UGaiaInventoryRPCComponent::GetInventorySubsystem()
{
	if (!CachedSubsystem)
	{
		if (UWorld* World = GetWorld())
		{
			CachedSubsystem = World->GetSubsystem<UGaiaInventorySubsystem>();
		}
	}
	return CachedSubsystem;
}

APlayerController* UGaiaInventoryRPCComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

// ========================================
// 调试辅助函数
// ========================================

TArray<FGuid> UGaiaInventoryRPCComponent::GetCachedItemUIDs() const
{
	TArray<FGuid> ItemUIDs;
	CachedItems.GetKeys(ItemUIDs);
	return ItemUIDs;
}

TArray<FGuid> UGaiaInventoryRPCComponent::GetCachedContainerUIDs() const
{
	TArray<FGuid> ContainerUIDs;
	CachedContainers.GetKeys(ContainerUIDs);
	return ContainerUIDs;
}

