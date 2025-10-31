#pragma once

#include "CoreMinimal.h"
#include "GameUIManagerSubsystem.h"
#include "GameplayTagContainer.h"
#include "GaiaUIManagerSubsystem.generated.h"

class UGaiaPrimaryGameLayout;
class UGaiaContainerWindowWidget;
class UCommonActivatableWidget;

/**
 * Gaia UI管理器（基于CommonUI Layer System）
 * 
 * 职责：
 * - 管理游戏所有UI的创建和显示（库存、对话、任务等）
 * - 使用PrimaryGameLayout的Layer System管理UI层级
 * - 提供便捷的API用于各系统的UI管理
 * 
 * 设计理念：
 * - 完全使用CommonUI的Layer System
 * - 统一管理所有游戏UI，而非仅限于库存系统
 * - Layer System自动处理Z-Order、激活/停用、ESC键
 * 
 * 扩展方向：
 * - 添加对话系统UI
 * - 添加任务系统UI
 * - 添加商店/交易UI
 * - 添加角色/装备UI
 */
UCLASS(config = Game)
class GAIAGAME_API UGaiaUIManagerSubsystem : public UGameUIManagerSubsystem
{
	GENERATED_BODY()

public:
	UGaiaUIManagerSubsystem();

	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface

	/**
	 * 获取Gaia UI管理器实例
	 * @param WorldContextObject 世界上下文对象
	 * @return UI管理器实例
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI", meta = (WorldContext = "WorldContextObject"))
	static UGaiaUIManagerSubsystem* Get(const UObject* WorldContextObject);

	// ========================================
	// 通用Layer操作（所有UI系统通用）
	// ========================================

	/**
	 * 推入Widget到指定Layer（通过类）
	 * @param LayerTag Layer标签
	 * @param WidgetClass Widget类
	 * @return Widget实例
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI")
	UCommonActivatableWidget* PushWidgetToLayer(
		FGameplayTag LayerTag,
		TSubclassOf<UCommonActivatableWidget> WidgetClass
	);

	/**
	 * 推入Widget到指定Layer（带初始化回调）
	 * @param LayerTag Layer标签
	 * @param WidgetClass Widget类
	 * @param InitFunc 初始化回调函数
	 * @return 创建的Widget实例
	 */
	template<typename WidgetT = UCommonActivatableWidget>
	WidgetT* PushWidgetToLayerWithInit(
		FGameplayTag LayerTag,
		TSubclassOf<WidgetT> WidgetClass,
		TFunctionRef<void(WidgetT&)> InitFunc
	)
	{
		UGaiaPrimaryGameLayout* Layout = GetPrimaryGameLayout();
		if (!Layout || !WidgetClass)
		{
			return nullptr;
		}

		return Layout->PushWidgetToLayerStack<WidgetT>(LayerTag, WidgetClass, InitFunc);
	}

	/**
	 * 从Layer移除Widget
	 * @param Widget 要移除的Widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI")
	void RemoveWidgetFromLayer(UCommonActivatableWidget* Widget);

	// ========================================
	// 库存系统UI（容器窗口管理）
	// ========================================

	/**
	 * 打开容器窗口
	 * @param ContainerUID 容器UID
	 * @param bSuspendInputUntilComplete 加载时是否暂停输入
	 * @return 窗口Widget实例
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|Inventory")
	UGaiaContainerWindowWidget* OpenContainerWindow(
		const FGuid& ContainerUID,
		bool bSuspendInputUntilComplete = false
	);

	/**
	 * 关闭指定容器窗口
	 * @param Widget 要关闭的窗口Widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|Inventory")
	void CloseContainerWindow(UGaiaContainerWindowWidget* Widget);

	/**
	 * 关闭所有容器窗口
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|Inventory")
	void CloseAllContainerWindows();

	/**
	 * 检查容器窗口是否已打开
	 * @param ContainerUID 容器UID
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|Inventory")
	bool IsContainerWindowOpen(const FGuid& ContainerUID) const;

	/**
	 * 通过物品UID打开容器（右键菜单调用）
	 * 自动查找物品拥有的容器并打开
	 * @param ItemUID 物品UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|UI|Inventory")
	void OpenContainerByItemUID(const FGuid& ItemUID);

protected:
	/**
	 * 获取主玩家的PrimaryGameLayout
	 */
	UGaiaPrimaryGameLayout* GetPrimaryGameLayout() const;
	
	/**
	 * 库存更新回调（由 RPC 组件的 OnInventoryUpdated 事件触发）
	 * 刷新所有打开的容器窗口
	 */
	UFUNCTION()
	void OnInventoryUpdated();
	
	/**
	 * 绑定库存事件（从主玩家的 RPC 组件）
	 */
	void BindInventoryEvents();

protected:
	// ========================================
	// 配置
	// ========================================

	/** 容器窗口Widget类 */
	UPROPERTY(config, EditDefaultsOnly, Category = "Gaia|Inventory|UI")
	TSoftClassPtr<UGaiaContainerWindowWidget> ContainerWindowClass;

	// ========================================
	// 运行时数据
	// ========================================

	/** 当前打开的容器窗口映射（ContainerUID -> Widget） */
	UPROPERTY(Transient)
	TMap<FGuid, TObjectPtr<UGaiaContainerWindowWidget>> OpenContainerWindows;
};
