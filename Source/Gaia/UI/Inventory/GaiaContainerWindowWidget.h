#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GaiaContainerWindowWidget.generated.h"

class UGaiaContainerGridWidget;
class UGaiaContainerDebugInfoWidget;
class UTextBlock;
class UButton;
class UBorder;

/**
 * 容器窗口Widget（重构为CommonActivatableWidget）
 * 
 * 职责：
 * - 显示单个容器的UI
 * - 支持关闭窗口（自动处理ESC键）
 * - 显示调试信息（测试模式）
 * - 自动管理激活/停用状态
 * 
 * 设计理念：
 * - 使用CommonUI的ActivatableWidget生命周期
 * - 由PrimaryGameLayout的Layer System管理
 * - 自动处理输入路由和焦点管理
 */
UCLASS()
class GAIAGAME_API UGaiaContainerWindowWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	//~ End UUserWidget Interface
	
	//~ Begin UCommonActivatableWidget Interface
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	//~ End UCommonActivatableWidget Interface

	// ========================================
	// 窗口管理
	// ========================================

	/**
	 * 初始化窗口
	 * @param InContainerUID 容器UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|UI")
	void InitializeWindow(const FGuid& InContainerUID);

	/**
	 * 请求关闭窗口（停用Widget，由Layer System管理）
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|UI")
	void RequestClose();

	/**
	 * 获取容器UID
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory|UI")
	FGuid GetContainerUID() const { return ContainerUID; }
	
	/**
	 * 获取容器网格Widget
	 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory|UI")
	UGaiaContainerGridWidget* GetContainerGrid() const { return ContainerGrid; }

	// ========================================
	// 调试功能
	// ========================================

	/**
	 * 设置调试模式
	 * @param bEnabled 是否启用调试模式
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|UI")
	void SetDebugMode(bool bEnabled);

	/**
	 * 刷新调试信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|UI")
	void RefreshDebugInfo();

protected:
	/**
	 * 关闭按钮点击
	 */
	UFUNCTION()
	void OnCloseButtonClicked();

protected:
	/** 容器UID */
	UPROPERTY(BlueprintReadOnly, Category = "Gaia|Inventory|UI")
	FGuid ContainerUID;

	/** 是否处于调试模式 */
	UPROPERTY(BlueprintReadWrite, Category = "Gaia|Inventory|UI")
	bool bDebugMode = false;

	// ========================================
	// Widget绑定（需要在UMG中绑定）
	// ========================================

	/** 标题栏 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		TObjectPtr<UBorder> Border_TitleBar;

	/** 标题文本 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Title;

	/** 关闭按钮 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> Button_Close;

	/** 容器网格 */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UGaiaContainerGridWidget> ContainerGrid;

	/** 调试信息面板（可选） */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UGaiaContainerDebugInfoWidget> DebugInfoPanel;
};

