#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GaiaContainerGridWidget.generated.h"

class UGaiaItemSlotWidget;
class UUniformGridPanel;
class UWrapBox;

/**
 * 容器网格Widget
 * 
 * 职责：
 * - 显示容器的槽位网格
 * - 管理槽位Widget
 * - 自动创建和更新槽位
 * - 支持网格布局或自动换行布局
 */
UCLASS()
class GAIAGAME_API UGaiaContainerGridWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	//~ End UUserWidget Interface

	// ========================================
	// 容器管理
	// ========================================

	/**
	 * 初始化容器
	 * @param InContainerUID 容器UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|UI")
	void InitializeContainer(const FGuid& InContainerUID);

	/**
	 * 刷新所有槽位（从InventorySubsystem获取最新数据）
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|UI")
	void RefreshAllSlots();

	/**
	 * 刷新指定槽位
	 * @param SlotID 槽位ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|UI")
	void RefreshSlot(int32 SlotID);

	/**
	 * 清空所有槽位
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|UI")
	void ClearAllSlots();

	/**
	 * 获取容器UID
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory|UI")
	FGuid GetContainerUID() const { return ContainerUID; }

	// ========================================
	// 槽位访问
	// ========================================

	/**
	 * 获取指定槽位Widget
	 * @param SlotID 槽位ID
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory|UI")
	UGaiaItemSlotWidget* GetSlotWidget(int32 SlotID) const;

	/**
	 * 获取所有槽位Widget
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory|UI")
	TArray<UGaiaItemSlotWidget*> GetAllSlotWidgets() const { return SlotWidgets; }

protected:
	/** 创建所有槽位Widget */
	void CreateSlotWidgets();

	/** 销毁所有槽位Widget */
	void DestroySlotWidgets();

protected:
	/** 容器UID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|Inventory|UI")
	FGuid ContainerUID;

	/** 所有槽位Widget */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|Inventory|UI")
	TArray<TObjectPtr<UGaiaItemSlotWidget>> SlotWidgets;

	// ========================================
	// Widget绑定（在UMG中选择一种布局方式）
	// ========================================

	/** 网格面板（优先使用，固定网格布局） */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UUniformGridPanel> GridPanel;

	/** 换行容器（备选，自动换行布局） */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UWrapBox> WrapBox;

	// ========================================
	// 配置
	// ========================================

	/** 物品槽位Widget类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Inventory|UI")
	TSubclassOf<UGaiaItemSlotWidget> ItemSlotWidgetClass;

	/** 网格列数（仅用于GridPanel） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Inventory|UI")
	int32 GridColumns = 10;

	/** 槽位间距 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Inventory|UI")
	FVector2D SlotPadding = FVector2D(2.0f, 2.0f);
};

