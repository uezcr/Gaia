// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PrimaryGameLayout.h"
#include "GameplayTagContainer.h"
#include "GaiaPrimaryGameLayout.generated.h"

class UCommonActivatableWidgetContainerBase;
class UCommonActivatableWidgetStack;

/**
 * Gaia游戏的主UI布局
 * 
 * 职责：
 * - 管理UI层级系统（Game、Container、Menu、Modal）
 * - 提供便捷的API推入/移除Widget
 * - 自动管理Widget的激活/停用和Z-Order
 * 
 * 使用方式：
 * 1. 在UMG中创建蓝图，继承此类
 * 2. 添加4个CommonActivatableWidgetStack，分别绑定到Layer变量
 * 3. 在代码中通过GameplayTag推入Widget
 */
UCLASS(Abstract, Blueprintable)
class GAIAGAME_API UGaiaPrimaryGameLayout : public UPrimaryGameLayout
{
	GENERATED_BODY()

public:
	UGaiaPrimaryGameLayout(const FObjectInitializer& ObjectInitializer);

	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	//~ End UUserWidget Interface

	// ========================================
	// Layer访问器
	// ========================================

	/** 获取游戏层（背包、装备等主要游戏UI） */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|Layout")
	UCommonActivatableWidgetStack* GetGameLayer() const { return GameLayer; }

	/** 获取容器层（多个容器窗口） */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|Layout")
	UCommonActivatableWidgetStack* GetContainerLayer() const { return ContainerLayer; }

	/** 获取菜单层（右键菜单、Tooltip） */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|Layout")
	UCommonActivatableWidgetStack* GetMenuLayer() const { return MenuLayer; }

	/** 获取模态层（对话框） */
	UFUNCTION(BlueprintPure, Category = "Gaia|UI|Layout")
	UCommonActivatableWidgetStack* GetModalLayer() const { return ModalLayer; }

	// ========================================
	// Layer Tags（用于代码访问）
	// ========================================

	/** UI.Layer.Inventory.Game */
	static FGameplayTag GetGameLayerTag();

	/** UI.Layer.Inventory.Container */
	static FGameplayTag GetContainerLayerTag();

	/** UI.Layer.Inventory.Menu */
	static FGameplayTag GetMenuLayerTag();

	/** UI.Layer.Inventory.Modal */
	static FGameplayTag GetModalLayerTag();

protected:
	// ========================================
	// Layer Widgets（在UMG中绑定）
	// ========================================

	/** 游戏层 - 背包、装备等主界面 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> GameLayer;

	/** 容器层 - 多个容器窗口可以同时打开 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> ContainerLayer;

	/** 菜单层 - 右键菜单、Tooltip等 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuLayer;

	/** 模态层 - 对话框（数量输入、确认等） */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> ModalLayer;

private:
	/** 注册所有Layer */
	void RegisterAllLayers();
};

