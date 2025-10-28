// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaItemDragDropOperation.h"
#include "GaiaItemSlotWidget.h"
#include "Gameplay/Inventory/GaiaInventorySubsystem.h"
#include "Gameplay/Inventory/GaiaInventoryRPCComponent.h"
#include "Player/GaiaPlayerController.h"
#include "GaiaLogChannels.h"

UGaiaItemDragDropOperation* UGaiaItemDragDropOperation::CreateDragDropOperation(UGaiaItemSlotWidget* SourceSlot)
{
	if (!SourceSlot || SourceSlot->IsEmpty())
	{
		UE_LOG(LogGaia, Error, TEXT("[拖放操作] 无效的源槽位或槽位为空"));
		return nullptr;
	}
	
	UGaiaItemDragDropOperation* Operation = NewObject<UGaiaItemDragDropOperation>();
	if (!Operation)
	{
		UE_LOG(LogGaia, Error, TEXT("[拖放操作] 无法创建拖放操作"));
		return nullptr;
	}
	
	// 设置拖放数据
	Operation->SourceSlotWidget = SourceSlot;
	Operation->SourceContainerUID = SourceSlot->GetContainerUID();
	Operation->SourceSlotID = SourceSlot->GetSlotID();
	Operation->ItemUID = SourceSlot->GetItemUID();
	
	// 获取物品信息
	UGaiaInventorySubsystem* InvSys = UGaiaInventorySubsystem::Get(SourceSlot->GetWorld());
	if (InvSys)
	{
		FGaiaItemInstance ItemInstance;
		if (InvSys->FindItemByUID(Operation->ItemUID, ItemInstance))
		{
			Operation->ItemDefinitionID = ItemInstance.ItemDefinitionID;
			Operation->Quantity = ItemInstance.Quantity;
		}
	}
	
	// TODO: 创建拖放视觉Widget
	// Operation->DefaultDragVisual = ...;
	
	UE_LOG(LogGaia, Log, TEXT("[拖放操作] 创建: Item=%s, Container=%s, Slot=%d, Qty=%d"),
		*Operation->ItemUID.ToString(),
		*Operation->SourceContainerUID.ToString(),
		Operation->SourceSlotID,
		Operation->Quantity);
	
	return Operation;
}

bool UGaiaItemDragDropOperation::ExecuteDropToSlot(UGaiaItemSlotWidget* TargetSlot)
{
	UE_LOG(LogGaia, Warning, TEXT("[拖放操作] ⭐ ExecuteDropToSlot 被调用"));
	
	if (!TargetSlot)
	{
		UE_LOG(LogGaia, Error, TEXT("[拖放操作] ❌ 目标槽位无效"));
		return false;
	}
	
	UE_LOG(LogGaia, Warning, TEXT("[拖放操作] 源: Container=%s, Slot=%d, Item=%s"),
		*SourceContainerUID.ToString(), SourceSlotID, *ItemUID.ToString());
	UE_LOG(LogGaia, Warning, TEXT("[拖放操作] 目标: Container=%s, Slot=%d, IsEmpty=%d"),
		*TargetSlot->GetContainerUID().ToString(), TargetSlot->GetSlotID(), TargetSlot->IsEmpty());
	
	// 检查是否可以拖放
	FText ErrorMessage;
	if (!CanDropToSlot(TargetSlot, ErrorMessage))
	{
		UE_LOG(LogGaia, Warning, TEXT("[拖放操作] ❌ 无法拖放: %s"), *ErrorMessage.ToString());
		return false;
	}
	
	// 检查是否拖放到自己
	if (TargetSlot == SourceSlotWidget)
	{
		UE_LOG(LogGaia, Verbose, TEXT("[拖放操作] 拖放到自己，取消操作"));
		return false;
	}
	
	// 检查是否在同一个槽位
	if (TargetSlot->GetContainerUID() == SourceContainerUID && TargetSlot->GetSlotID() == SourceSlotID)
	{
		UE_LOG(LogGaia, Verbose, TEXT("[拖放操作] 拖放到相同槽位，取消操作"));
		return false;
	}
	
	// 判断操作类型
	if (TargetSlot->IsEmpty())
	{
		UE_LOG(LogGaia, Warning, TEXT("[拖放操作] → 移动到空槽位"));
		// 移动到空槽位
		return MoveItemToSlot(TargetSlot);
	}
	else if (TargetSlot->GetItemUID() == ItemUID)
	{
		// 同一个物品，不应该发生
		UE_LOG(LogGaia, Warning, TEXT("[拖放操作] ❌ 尝试拖放到同一个物品实例"));
		return false;
	}
	else
	{
		// 目标槽位有其他物品
		// 检查是否可以堆叠
		UGaiaInventorySubsystem* InvSys = UGaiaInventorySubsystem::Get(TargetSlot->GetWorld());
		if (InvSys)
		{
			FGaiaItemInstance TargetItem;
			if (InvSys->FindItemByUID(TargetSlot->GetItemUID(), TargetItem))
			{
				if (TargetItem.ItemDefinitionID == ItemDefinitionID)
				{
					UE_LOG(LogGaia, Warning, TEXT("[拖放操作] → 堆叠物品"));
					// 可以堆叠
					return StackItemToSlot(TargetSlot);
				}
			}
		}
		
		UE_LOG(LogGaia, Warning, TEXT("[拖放操作] → 交换物品"));
		// 不能堆叠，交换
		return SwapItemWithSlot(TargetSlot);
	}
}

bool UGaiaItemDragDropOperation::ExecuteDropToWorld(const FVector& WorldLocation)
{
	// TODO: 实现丢弃到世界的逻辑
	// 1. 在世界中生成物品Actor
	// 2. 从库存中移除物品
	
	UE_LOG(LogGaia, Log, TEXT("[拖放操作] 丢弃到世界: Location=%s"), *WorldLocation.ToString());
	
	return false;
}

void UGaiaItemDragDropOperation::CancelDragDrop()
{
	UE_LOG(LogGaia, Log, TEXT("[拖放操作] 取消拖放"));
	
	// 恢复源槽位视觉状态
	if (SourceSlotWidget)
	{
		SourceSlotWidget->SetHighlighted(false);
	}
}

bool UGaiaItemDragDropOperation::CanDropToSlot(UGaiaItemSlotWidget* TargetSlot, FText& OutErrorMessage) const
{
	if (!TargetSlot)
	{
		OutErrorMessage = FText::FromString(TEXT("目标槽位无效"));
		return false;
	}
	
	// 检查容器是否相同
	FGuid TargetContainerUID = TargetSlot->GetContainerUID();
	int32 TargetSlotID = TargetSlot->GetSlotID();
	
	// 如果是同一个槽位，不允许
	if (TargetContainerUID == SourceContainerUID && TargetSlotID == SourceSlotID)
	{
		OutErrorMessage = FText::FromString(TEXT("不能拖放到自己"));
		return false;
	}
	
	// TODO: 添加更多检查
	// - 容器权限检查
	// - 物品类型限制检查
	// - 槽位锁定检查
	
	OutErrorMessage = FText::GetEmpty();
	return true;
}

bool UGaiaItemDragDropOperation::MoveItemToSlot(UGaiaItemSlotWidget* TargetSlot)
{
	if (!TargetSlot || !TargetSlot->IsEmpty())
	{
		return false;
	}
	
	UWorld* World = TargetSlot->GetWorld();
	if (!World)
	{
		return false;
	}
	
	// 获取RPC组件
	APlayerController* PC = World->GetFirstPlayerController();
	AGaiaPlayerController* GaiaPC = Cast<AGaiaPlayerController>(PC);
	if (!GaiaPC)
	{
		UE_LOG(LogGaia, Error, TEXT("[拖放操作] 无法获取GaiaPlayerController"));
		return false;
	}
	
	UGaiaInventoryRPCComponent* RPCComp = GaiaPC->GetInventoryRPCComponent();
	if (!RPCComp)
	{
		UE_LOG(LogGaia, Error, TEXT("[拖放操作] 无法获取RPC组件"));
		return false;
	}
	
	// 判断是否跨容器移动
	FGuid TargetContainerUID = TargetSlot->GetContainerUID();
	int32 TargetSlotID = TargetSlot->GetSlotID();
	
	if (TargetContainerUID == SourceContainerUID)
	{
		// 同容器内移动
		UE_LOG(LogGaia, Log, TEXT("[拖放操作] 容器内移动: Item=%s, FromSlot=%d, ToSlot=%d"),
			*ItemUID.ToString(), SourceSlotID, TargetSlotID);
	}
	else
	{
		// 跨容器移动
		UE_LOG(LogGaia, Log, TEXT("[拖放操作] 跨容器移动: Item=%s, FromContainer=%s, ToContainer=%s, ToSlot=%d"),
			*ItemUID.ToString(),
			*SourceContainerUID.ToString(),
			*TargetContainerUID.ToString(),
			TargetSlotID);
	}
	
	// 执行移动（系统会自动从ItemUID找到源容器）
	RPCComp->RequestMoveItem(
		ItemUID,
		TargetContainerUID,
		TargetSlotID
	);
	
	return true;
}

bool UGaiaItemDragDropOperation::SwapItemWithSlot(UGaiaItemSlotWidget* TargetSlot)
{
	if (!TargetSlot || TargetSlot->IsEmpty())
	{
		return false;
	}
	
	UWorld* World = TargetSlot->GetWorld();
	if (!World)
	{
		return false;
	}
	
	// 获取RPC组件
	APlayerController* PC = World->GetFirstPlayerController();
	AGaiaPlayerController* GaiaPC = Cast<AGaiaPlayerController>(PC);
	if (!GaiaPC)
	{
		return false;
	}
	
	UGaiaInventoryRPCComponent* RPCComp = GaiaPC->GetInventoryRPCComponent();
	if (!RPCComp)
	{
		return false;
	}
	
	FGuid TargetContainerUID = TargetSlot->GetContainerUID();
	int32 TargetSlotID = TargetSlot->GetSlotID();
	
	UE_LOG(LogGaia, Log, TEXT("[拖放操作] 交换物品: Item1=%s (Slot=%d), Item2=%s (Slot=%d)"),
		*ItemUID.ToString(), SourceSlotID,
		*TargetSlot->GetItemUID().ToString(), TargetSlotID);
	
	// 使用MoveItem实现交换（服务器会自动处理，系统会从ItemUID找到源容器）
	RPCComp->RequestMoveItem(
		ItemUID,
		TargetContainerUID,
		TargetSlotID
	);
	
	return true;
}

bool UGaiaItemDragDropOperation::StackItemToSlot(UGaiaItemSlotWidget* TargetSlot)
{
	if (!TargetSlot || TargetSlot->IsEmpty())
	{
		return false;
	}
	
	UWorld* World = TargetSlot->GetWorld();
	if (!World)
	{
		return false;
	}
	
	// 获取RPC组件
	APlayerController* PC = World->GetFirstPlayerController();
	AGaiaPlayerController* GaiaPC = Cast<AGaiaPlayerController>(PC);
	if (!GaiaPC)
	{
		return false;
	}
	
	UGaiaInventoryRPCComponent* RPCComp = GaiaPC->GetInventoryRPCComponent();
	if (!RPCComp)
	{
		return false;
	}
	
	FGuid TargetContainerUID = TargetSlot->GetContainerUID();
	int32 TargetSlotID = TargetSlot->GetSlotID();
	
	UE_LOG(LogGaia, Log, TEXT("[拖放操作] 堆叠物品: Source=%s (Qty=%d), Target=%s"),
		*ItemUID.ToString(), Quantity,
		*TargetSlot->GetItemUID().ToString());
	
	// 使用MoveItem实现堆叠（服务器会自动处理，系统会从ItemUID找到源容器）
	RPCComp->RequestMoveItem(
		ItemUID,
		TargetContainerUID,
		TargetSlotID
	);
	
	return true;
}

