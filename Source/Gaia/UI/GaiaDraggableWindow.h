// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GaiaDraggableWindow.generated.h"

class UGaiaDraggableTitleBar;
class UBorder;
class UNamedSlot;

/**
 * 可拖拽窗口基类
 * 
 * 特性：
 * - 使用 GaiaDraggableTitleBar 实现拖动
 * - 自动设置窗口大小和位置
 * - 支持居中显示
 * - 内容区域使用 NamedSlot，可在Blueprint中自定义
 * - 完全可配置的窗口样式
 * 
 * 使用方法：
 * 1. 创建Blueprint继承此类
 * 2. 在UMG中组件会自动创建（BindWidget）：
 *    - WindowBorder (Border) - 窗口边框
 *    - TitleBar (GaiaDraggableTitleBar) - 标题栏
 *    - ContentSlot (NamedSlot) - 内容区域
 * 3. 在ContentSlot中添加你的内容
 * 4. 配置窗口属性（大小、位置、标题等）
 * 
 * 示例：
 * ```cpp
 * // C++创建窗口
 * UGaiaDraggableWindow* Window = CreateWidget<UGaiaDraggableWindow>(GetWorld(), WindowClass);
 * Window->SetWindowTitle(NSLOCTEXT("UI", "Title", "My Window"));
 * Window->SetWindowSize(FVector2D(800, 600));
 * Window->ShowWindow();
 * 
 * // Blueprint使用
 * - 创建Widget Blueprint，父类选择 GaiaDraggableWindow
 * - 在ContentSlot中添加你的UI内容
 * - 设置Default Window Size等属性
 * - 调用 Show Window
 * ```
 */
UCLASS(Abstract, Blueprintable)
class GAIAGAME_API UGaiaDraggableWindow : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UGaiaDraggableWindow(const FObjectInitializer& ObjectInitializer);

	//~Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	//~End UUserWidget Interface

	// ========================================
	// 公开接口
	// ========================================

	/** 显示窗口（添加到视口并设置位置） - 已过时，请使用 ShowInCanvas */
	UFUNCTION(BlueprintCallable, Category = "Window", meta = (DeprecatedFunction, DeprecationMessage = "请使用 ShowInCanvas 将窗口添加到Canvas中"))
	void ShowWindow();

	/** 显示窗口（添加到指定Canvas并设置位置） */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void ShowInCanvas(class UCanvasPanel* Canvas, FVector2D Position = FVector2D(-1, -1));

	/** 关闭窗口（从父级移除） */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void CloseWindow();

	/** 设置窗口标题 */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void SetWindowTitle(const FText& InTitle);

	/** 获取窗口标题 */
	UFUNCTION(BlueprintPure, Category = "Window")
	FText GetWindowTitle() const;

	/** 设置窗口大小 */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void SetWindowSize(FVector2D InSize);

	/** 获取窗口大小 */
	UFUNCTION(BlueprintPure, Category = "Window")
	FVector2D GetWindowSize() const { return DefaultWindowSize; }

	/** 设置窗口位置 */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void SetWindowPosition(FVector2D InPosition);

	/** 获取窗口位置 */
	UFUNCTION(BlueprintPure, Category = "Window")
	FVector2D GetWindowPosition() const;

	/** 居中显示窗口 */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void CenterWindow();

	/** 设置是否显示关闭按钮 */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void SetCloseButtonVisible(bool bVisible);

	/** 设置是否可拖动 */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void SetDraggable(bool bCanDrag);

	/** 将窗口置顶（移动到Canvas的最上层） */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void BringToFront();

	// ========================================
	// 事件委托
	// ========================================

	/** 窗口关闭事件 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWindowClosed);
	UPROPERTY(BlueprintAssignable, Category = "Window|Events")
	FOnWindowClosed OnWindowClosed;

	/** 窗口显示事件 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWindowShown);
	UPROPERTY(BlueprintAssignable, Category = "Window|Events")
	FOnWindowShown OnWindowShown;

protected:
	// ========================================
	// Widget组件（需要在UMG中绑定）
	// ========================================

	/** 窗口边框 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> WindowBorder;

	/** 标题栏 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGaiaDraggableTitleBar> TitleBar;

	/** 内容槽位（在Blueprint中添加内容） */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UNamedSlot> ContentSlot;

	// ========================================
	// 可配置属性
	// ========================================

	/** 默认窗口标题 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Appearance")
	FText DefaultWindowTitle;

	/** 默认窗口大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Layout")
	FVector2D DefaultWindowSize;

	/** 默认窗口位置（-1表示居中） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Layout")
	FVector2D DefaultWindowPosition;

	/** 是否在显示时自动居中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Layout")
	bool bAutoCenterOnShow;

	/** 最小窗口大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Layout", meta = (ClampMin = "100"))
	FVector2D MinWindowSize;

	/** 最大窗口大小（0表示不限制） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Layout")
	FVector2D MaxWindowSize;

	/** 是否显示关闭按钮 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Appearance")
	bool bShowCloseButton;

	/** 窗口边框颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Appearance")
	FLinearColor WindowBorderColor;

	/** 窗口边框宽度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Appearance", meta = (ClampMin = "0", ClampMax = "10"))
	float WindowBorderThickness;

	/** 是否保存窗口位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Behavior")
	bool bSaveWindowPosition;

	/** 窗口位置保存Key（用于保存/加载） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window|Behavior", meta = (EditCondition = "bSaveWindowPosition"))
	FString WindowPositionSaveKey;

	// ========================================
	// 内部函数
	// ========================================

	/** 初始化窗口 */
	void InitializeWindow();

	/** 设置窗口样式 */
	void ApplyWindowStyle() const;

	/** 加载保存的窗口位置 */
	void LoadWindowPosition() const;

	/** 保存窗口位置 */
	void SaveWindowPosition() const;

	/** 限制窗口大小在最小/最大范围内 */
	FVector2D ClampWindowSize(const FVector2D& Size) const;

	/** 标题栏关闭按钮点击处理 */
	UFUNCTION()
	void HandleTitleBarCloseClicked();

	/** 标题栏拖动结束处理（用于保存位置） */
	UFUNCTION()
	void HandleTitleBarDragEnded();

private:
	/** 是否已经显示 */
	bool bIsShown;
};

