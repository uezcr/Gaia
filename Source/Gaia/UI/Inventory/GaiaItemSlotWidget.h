// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Gameplay/Inventory/GaiaInventoryTypes.h"
#include "GaiaItemSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;
class USizeBox;

/**
 * 物品槽位Widget
 * 
 * 职责：
 * - 显示单个物品槽位
 * - 显示物品图标、数量、品质等
 * - 处理鼠标悬停、点击、拖放
 * - 支持空槽位显示
 * 
 * 使用场景：
 * - 容器网格中的每个槽位
 * - 装备栏槽位
 * - 快捷栏槽位
 */
UCLASS()
class GAIAGAME_API UGaiaItemSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	//~ End UUserWidget Interface

	// ========================================
	// 槽位管理
	// ========================================

	/**
	 * 初始化槽位
	 * @param InContainerUID 所属容器UID
	 * @param InSlotID 槽位ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|ItemSlot")
	void InitializeSlot(const FGuid& InContainerUID, int32 InSlotID);

	/**
	 * 刷新槽位显示（从InventorySubsystem获取最新数据）
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|ItemSlot")
	void RefreshSlot();

	/**
	 * 设置槽位为空
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|ItemSlot")
	void SetEmpty();

	/**
	 * 设置槽位数据
	 * @param ItemInstance 物品实例数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|ItemSlot")
	void SetSlotData(const FGaiaItemInstance& ItemInstance);

	/**
	 * 获取槽位是否为空
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|ItemSlot")
	bool IsEmpty() const { return !ItemUID.IsValid(); }

	/**
	 * 获取容器UID
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|ItemSlot")
	FGuid GetContainerUID() const { return ContainerUID; }

	/**
	 * 获取槽位ID
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|ItemSlot")
	int32 GetSlotID() const { return SlotID; }

	/**
	 * 获取物品UID
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|ItemSlot")
	FGuid GetItemUID() const { return ItemUID; }

	// ========================================
	// 视觉状态
	// ========================================

	/**
	 * 设置高亮状态
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|ItemSlot")
	void SetHighlighted(bool bHighlight);

	/**
	 * 设置选中状态
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|ItemSlot")
	void SetSelected(bool bSelect);

	/**
	 * 设置可拖放目标状态
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|ItemSlot")
	void SetDropTarget(bool bIsTarget);

protected:
	// ========================================
	// 蓝图事件
	// ========================================

	/**
	 * 右键点击事件
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gaia|UI|ItemSlot")
	void OnRightClick();

	/**
	 * Shift+点击事件（快速移动）
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gaia|UI|ItemSlot")
	void OnShiftClick();

	/**
	 * Ctrl+点击事件（拆分）
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gaia|UI|ItemSlot")
	void OnCtrlClick();

protected:
	/** 所属容器UID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|ItemSlot")
	FGuid ContainerUID;

	/** 槽位ID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|ItemSlot")
	int32 SlotID = -1;

	/** 物品UID（为空表示空槽位） */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|ItemSlot")
	FGuid ItemUID;

	/** 物品定义ID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|ItemSlot")
	FName ItemDefinitionID = NAME_None;

	/** 物品数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|ItemSlot")
	int32 Quantity = 0;

	/** 是否高亮 */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|ItemSlot")
	bool bIsHighlighted = false;

	/** 是否选中 */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|ItemSlot")
	bool bIsSelected = false;

	/** 是否为拖放目标 */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|UI|ItemSlot")
	bool bIsDropTarget = false;

	// ========================================
	// Widget绑定（在UMG中绑定）
	// ========================================

	/** 槽位背景边框 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UBorder> Border_Background;

	/** 物品图标 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	/** 数量文本 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Quantity;

	/** 槽位大小容器（可选，用于固定槽位大小） */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBox_Slot;

	// ========================================
	// 样式配置
	// ========================================

	/** 空槽位颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|UI|ItemSlot|Style")
	FLinearColor EmptySlotColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.5f);

	/** 正常槽位颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|UI|ItemSlot|Style")
	FLinearColor NormalSlotColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);

	/** 高亮槽位颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|UI|ItemSlot|Style")
	FLinearColor HighlightSlotColor = FLinearColor(0.5f, 0.5f, 0.0f, 1.0f);

	/** 选中槽位颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|UI|ItemSlot|Style")
	FLinearColor SelectedSlotColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	/** 拖放目标槽位颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|UI|ItemSlot|Style")
	FLinearColor DropTargetSlotColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	/** 槽位大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|UI|ItemSlot|Style")
	float SlotSize = 64.0f;

private:
	/** 更新槽位视觉 */
	void UpdateSlotVisuals();

	/** 加载物品图标 */
	void LoadItemIcon(const FGaiaItemDefinition& ItemDef);
};

