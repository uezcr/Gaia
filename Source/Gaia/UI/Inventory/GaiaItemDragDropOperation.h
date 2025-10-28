// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "GaiaItemDragDropOperation.generated.h"

class UGaiaItemSlotWidget;

/**
 * 物品拖放操作
 * 
 * 职责：
 * - 存储拖放的物品信息
 * - 提供拖放视觉反馈
 * - 执行拖放完成后的操作
 */
UCLASS()
class GAIAGAME_API UGaiaItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/**
	 * 创建拖放操作
	 * @param SourceSlot 源槽位Widget
	 * @return 拖放操作实例
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|DragDrop")
	static UGaiaItemDragDropOperation* CreateDragDropOperation(UGaiaItemSlotWidget* SourceSlot);

	// ========================================
	// 拖放数据
	// ========================================

	/** 源槽位Widget */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|DragDrop")
	TObjectPtr<UGaiaItemSlotWidget> SourceSlotWidget;

	/** 源容器UID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|DragDrop")
	FGuid SourceContainerUID;

	/** 源槽位ID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|DragDrop")
	int32 SourceSlotID = -1;

	/** 被拖放的物品UID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|DragDrop")
	FGuid ItemUID;

	/** 物品定义ID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|DragDrop")
	FName ItemDefinitionID = NAME_None;

	/** 物品数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|DragDrop")
	int32 Quantity = 0;

	/** 是否是拆分操作（Ctrl拖放） */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|DragDrop")
	bool bIsSplitting = false;

	/** 拆分数量（仅在bIsSplitting为true时有效） */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|DragDrop")
	int32 SplitQuantity = 0;

	// ========================================
	// 拖放操作
	// ========================================

	/**
	 * 执行拖放到槽位
	 * @param TargetSlot 目标槽位Widget
	 * @return 是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|DragDrop")
	bool ExecuteDropToSlot(UGaiaItemSlotWidget* TargetSlot);

	/**
	 * 执行拖放到世界（丢弃）
	 * @param WorldLocation 世界位置
	 * @return 是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|DragDrop")
	bool ExecuteDropToWorld(const FVector& WorldLocation);

	/**
	 * 取消拖放操作
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|DragDrop")
	void CancelDragDrop();

	/**
	 * 检查是否可以拖放到指定槽位
	 * @param TargetSlot 目标槽位Widget
	 * @param OutErrorMessage 错误信息
	 * @return 是否可以拖放
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|DragDrop")
	bool CanDropToSlot(UGaiaItemSlotWidget* TargetSlot, FText& OutErrorMessage) const;

protected:
	/**
	 * 移动物品到目标槽位
	 */
	bool MoveItemToSlot(UGaiaItemSlotWidget* TargetSlot);

	/**
	 * 交换物品槽位
	 */
	bool SwapItemWithSlot(UGaiaItemSlotWidget* TargetSlot);

	/**
	 * 堆叠物品到目标槽位
	 */
	bool StackItemToSlot(UGaiaItemSlotWidget* TargetSlot);
};

