// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GaiaDraggableTitleBar.generated.h"

class UCommonTextBlock;
class UCommonButtonBase;
class UBorder;

/**
 * 可拖动标题栏基类
 * 
 * 功能：
 * - 显示标题文本
 * - 显示关闭按钮
 * - 支持拖动父Widget
 * - 自定义样式
 * 
 * 使用方法：
 * 1. 创建Blueprint继承此类
 * 2. 在UMG中添加组件：
 *    - TitleBarBorder (Border)
 *    - TitleText (CommonTextBlock)
 *    - CloseButton (CommonButtonBase)
 * 3. 绑定这些组件（BindWidget）
 * 4. 父Widget会自动获得拖动能力
 * 
 * 示例：
 * ```
 * // C++
 * TitleBar->SetTitleText(NSLOCTEXT("UI", "WindowTitle", "My Window"));
 * TitleBar->OnCloseClicked.AddDynamic(this, &UMyWindow::HandleClose);
 * 
 * // Blueprint
 * Set Title Text -> "My Window"
 * On Close Clicked -> Handle Close Event
 * ```
 */
UCLASS(Abstract, Blueprintable)
class GAIAGAME_API UGaiaDraggableTitleBar : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UGaiaDraggableTitleBar(const FObjectInitializer& ObjectInitializer);

	//~Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	//~End UUserWidget Interface

	// ========================================
	// 公开接口
	// ========================================

	/** 设置标题文本 */
	UFUNCTION(BlueprintCallable, Category = "Title Bar")
	void SetTitleText(const FText& InTitleText);

	/** 获取标题文本 */
	UFUNCTION(BlueprintPure, Category = "Title Bar")
	FText GetTitleText() const;

	/** 设置是否显示关闭按钮 */
	UFUNCTION(BlueprintCallable, Category = "Title Bar")
	void SetCloseButtonVisible(bool bVisible);

	/** 获取是否正在拖动 */
	UFUNCTION(BlueprintPure, Category = "Title Bar")
	bool IsDragging() const { return bIsDragging; }

	/** 设置是否可拖动 */
	UFUNCTION(BlueprintCallable, Category = "Title Bar")
	void SetDraggable(bool bCanDrag) { bIsDraggable = bCanDrag; }

	/** 获取是否可拖动 */
	UFUNCTION(BlueprintPure, Category = "Title Bar")
	bool IsDraggable() const { return bIsDraggable; }

	// ========================================
	// 事件委托
	// ========================================

	/** 关闭按钮点击事件 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseClicked);
	UPROPERTY(BlueprintAssignable, Category = "Title Bar|Events")
	FOnCloseClicked OnCloseClicked;

	/** 开始拖动事件 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDragStarted);
	UPROPERTY(BlueprintAssignable, Category = "Title Bar|Events")
	FOnDragStarted OnDragStarted;

	/** 拖动结束事件 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDragEnded);
	UPROPERTY(BlueprintAssignable, Category = "Title Bar|Events")
	FOnDragEnded OnDragEnded;

	/** 双击标题栏事件（可用于最大化/恢复） */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTitleBarDoubleClicked);
	UPROPERTY(BlueprintAssignable, Category = "Title Bar|Events")
	FOnTitleBarDoubleClicked OnTitleBarDoubleClicked;

protected:
	// ========================================
	// Widget组件（需要在UMG中绑定）
	// ========================================

	/** 标题栏背景边框 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> TitleBarBorder;

	/** 标题文本 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> TitleText;

	/** 关闭按钮 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CloseButton;

	// ========================================
	// 可配置属性
	// ========================================

	/** 默认标题文本 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title Bar")
	FText DefaultTitleText;

	/** 是否可拖动 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title Bar")
	bool bIsDraggable;

	/** 是否显示关闭按钮 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title Bar")
	bool bShowCloseButton;

	/** 拖动时的透明度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title Bar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DragOpacity;

	/** 是否限制在屏幕内 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title Bar")
	bool bConstrainToScreen;

	// ========================================
	// 内部状态
	// ========================================

	/** 是否正在拖动 */
	UPROPERTY(BlueprintReadOnly, Category = "Title Bar")
	bool bIsDragging;

	/** 拖动偏移量 */
	FVector2D DragOffset;

	/** 原始透明度 */
	float OriginalOpacity;

	// ========================================
	// 内部函数
	// ========================================

	/** 关闭按钮点击处理 */
	UFUNCTION()
	void HandleCloseButtonClicked();

	/** 开始拖动 */
	void StartDragging(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);

	/** 更新拖动 */
	void UpdateDragging(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) const;

	/** 结束拖动 */
	void EndDragging();

	/** 获取父Widget */
	UUserWidget* GetParentWidget() const;

	/** 限制位置在屏幕内 */
	static FVector2D ConstrainPositionToScreen(const FVector2D& Position, const FVector2D& Size);
};

