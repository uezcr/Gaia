// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaDraggableWindow.h"
#include "GaiaDraggableTitleBar.h"
#include "Components/Border.h"
#include "Components/NamedSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"

UGaiaDraggableWindow::UGaiaDraggableWindow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 默认值
	DefaultWindowTitle = NSLOCTEXT("GaiaUI", "DefaultWindowTitle", "Window");
	DefaultWindowSize = FVector2D(800, 600);
	DefaultWindowPosition = FVector2D(-1, -1); // -1表示居中
	bAutoCenterOnShow = true;
	MinWindowSize = FVector2D(400, 300);
	MaxWindowSize = FVector2D(0, 0); // 0表示不限制
	bShowCloseButton = true;
	WindowBorderColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // 金色
	WindowBorderThickness = 2.0f;
	bSaveWindowPosition = false;
	bIsShown = false;
}

void UGaiaDraggableWindow::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeWindow();
}

void UGaiaDraggableWindow::NativeDestruct()
{
	// 保存窗口位置
	if (bSaveWindowPosition && bIsShown)
	{
		SaveWindowPosition();
	}

	// 清理标题栏事件绑定
	if (TitleBar)
	{
		TitleBar->OnCloseClicked.RemoveAll(this);
		TitleBar->OnDragEnded.RemoveAll(this);
	}

	Super::NativeDestruct();
}

FReply UGaiaDraggableWindow::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 窗口被点击时置顶
	BringToFront();

	// 继续传递事件给子控件
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UGaiaDraggableWindow::ShowWindow()
{
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] ShowWindow 已过时，请使用 ShowInCanvas"));
	
	if (bIsShown)
	{
		return;
	}

	// 添加到视口
	AddToViewport();

	// 设置对齐方式为左上角
	SetAlignmentInViewport(FVector2D(0.0f, 0.0f));

	// 限制窗口大小
	FVector2D ClampedSize = ClampWindowSize(DefaultWindowSize);
	SetDesiredSizeInViewport(ClampedSize);

	// 设置窗口位置
	if (bAutoCenterOnShow || (DefaultWindowPosition.X < 0 || DefaultWindowPosition.Y < 0))
	{
		CenterWindow();
	}
	else
	{
		// 加载保存的位置
		if (bSaveWindowPosition)
		{
			LoadWindowPosition();
		}
		else
		{
			SetPositionInViewport(DefaultWindowPosition, false);
		}
	}

	bIsShown = true;

	// 触发显示事件
	OnWindowShown.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[GaiaDraggableWindow] 窗口已显示: %s"), *GetWindowTitle().ToString());
}

void UGaiaDraggableWindow::ShowInCanvas(UCanvasPanel* Canvas, FVector2D Position)
{
	if (!Canvas)
	{
		UE_LOG(LogTemp, Error, TEXT("[GaiaDraggableWindow] ShowInCanvas: Canvas 为空！"));
		return;
	}

	if (bIsShown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] 窗口已显示"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[GaiaDraggableWindow] 将窗口添加到Canvas: %s"), *GetWindowTitle().ToString());

	// 添加到Canvas
	UCanvasPanelSlot* CanvasSlot = Canvas->AddChildToCanvas(this);
	if (!CanvasSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("[GaiaDraggableWindow] 无法添加到Canvas！"));
		return;
	}

	// 限制窗口大小
	FVector2D ClampedSize = ClampWindowSize(DefaultWindowSize);
	CanvasSlot->SetSize(ClampedSize);

	// 设置锚点为左上角
	CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
	CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));

	// 设置窗口位置
	FVector2D FinalPosition = Position;
	
	if (Position.X < 0 || Position.Y < 0 || bAutoCenterOnShow)
	{
		// 居中显示
		FVector2D CanvasSize = Canvas->GetCachedGeometry().GetLocalSize();
		FinalPosition = (CanvasSize - ClampedSize) * 0.5f;
		FinalPosition.X = FMath::Max(0.0f, FinalPosition.X);
		FinalPosition.Y = FMath::Max(0.0f, FinalPosition.Y);
		
		UE_LOG(LogTemp, Log, TEXT("[GaiaDraggableWindow] 居中显示: Position=(%.2f, %.2f)"), FinalPosition.X, FinalPosition.Y);
	}
	else if (bSaveWindowPosition && !WindowPositionSaveKey.IsEmpty())
	{
		// TODO: 加载保存的位置
		FinalPosition = Position;
	}

	CanvasSlot->SetPosition(FinalPosition);

	bIsShown = true;

	// 触发显示事件
	OnWindowShown.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[GaiaDraggableWindow] 窗口已添加到Canvas: 位置=(%.2f, %.2f), 大小=(%.2f, %.2f)"), 
		FinalPosition.X, FinalPosition.Y, ClampedSize.X, ClampedSize.Y);
}

void UGaiaDraggableWindow::CloseWindow()
{
	if (!bIsShown)
	{
		return;
	}

	// 保存窗口位置
	if (bSaveWindowPosition)
	{
		SaveWindowPosition();
	}

	// 从父级移除
	RemoveFromParent();

	bIsShown = false;

	// 触发关闭事件
	OnWindowClosed.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[GaiaDraggableWindow] 窗口已关闭: %s"), *GetWindowTitle().ToString());
}

void UGaiaDraggableWindow::SetWindowTitle(const FText& InTitle)
{
	DefaultWindowTitle = InTitle;

	if (TitleBar)
	{
		TitleBar->SetTitleText(InTitle);
	}
}

FText UGaiaDraggableWindow::GetWindowTitle() const
{
	if (TitleBar)
	{
		return TitleBar->GetTitleText();
	}

	return DefaultWindowTitle;
}

void UGaiaDraggableWindow::SetWindowSize(FVector2D InSize)
{
	// 限制大小
	DefaultWindowSize = ClampWindowSize(InSize);

	// 如果已经显示，更新视口中的大小
	if (bIsShown)
	{
		SetDesiredSizeInViewport(DefaultWindowSize);
	}
}

void UGaiaDraggableWindow::SetWindowPosition(FVector2D InPosition)
{
	DefaultWindowPosition = InPosition;

	// 如果已经显示，更新视口中的位置
	if (bIsShown)
	{
		SetPositionInViewport(InPosition, false);
	}
}

FVector2D UGaiaDraggableWindow::GetWindowPosition() const
{
	if (bIsShown)
	{
		return GetCachedGeometry().GetAbsolutePosition();
	}

	return DefaultWindowPosition;
}

void UGaiaDraggableWindow::CenterWindow()
{
	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	// 获取视口大小
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	// 计算居中位置
	FVector2D WindowSize = DefaultWindowSize;
	FVector2D Position = (ViewportSize - WindowSize) * 0.5f;

	// 确保不出屏幕
	Position.X = FMath::Max(0.0f, Position.X);
	Position.Y = FMath::Max(0.0f, Position.Y);

	// 设置位置
	if (bIsShown)
	{
		SetPositionInViewport(Position, false);
	}
	else
	{
		DefaultWindowPosition = Position;
	}

	UE_LOG(LogTemp, Verbose, TEXT("[GaiaDraggableWindow] 窗口居中: Position=(%.2f, %.2f)"), 
		Position.X, Position.Y);
}

void UGaiaDraggableWindow::SetCloseButtonVisible(bool bVisible)
{
	bShowCloseButton = bVisible;

	if (TitleBar)
	{
		TitleBar->SetCloseButtonVisible(bVisible);
	}
}

void UGaiaDraggableWindow::SetDraggable(bool bCanDrag)
{
	if (TitleBar)
	{
		TitleBar->SetDraggable(bCanDrag);
	}
}

void UGaiaDraggableWindow::BringToFront()
{
	if (!bIsShown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] 窗口未显示，无法置顶"));
		return;
	}

	// 获取Canvas Slot
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CanvasSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] 窗口不在Canvas中，无法置顶"));
		return;
	}

	// 获取Canvas Panel
	UCanvasPanel* Canvas = Cast<UCanvasPanel>(CanvasSlot->Parent);
	if (!Canvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] 无法获取Canvas，无法置顶"));
		return;
	}

	// 获取当前窗口在Canvas中的索引
	int32 CurrentIndex = Canvas->GetChildIndex(this);
	int32 ChildCount = Canvas->GetChildrenCount();

	// 如果已经在最上层，则不需要移动
	if (CurrentIndex == ChildCount - 1)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[GaiaDraggableWindow] 窗口已在最上层 (索引: %d/%d)"), CurrentIndex, ChildCount - 1);
		return;
	}

	// 使用 ReplaceChild 方法在不删除的情况下改变顺序
	// 先保存当前Slot的所有设置
	FVector2D CurrentPosition = CanvasSlot->GetPosition();
	FVector2D CurrentSize = CanvasSlot->GetSize();
	FAnchors CurrentAnchors = CanvasSlot->GetAnchors();
	FVector2D CurrentAlignment = CanvasSlot->GetAlignment();
	int32 CurrentZOrder = CanvasSlot->GetZOrder();

	// 由于UCanvasPanel没有直接移动子控件顺序的方法，我们需要使用删除后添加
	// 但这是在单个函数内完成的原子操作，不会有跨容器移动的问题
	Canvas->RemoveChild(this);
	UCanvasPanelSlot* NewSlot = Canvas->AddChildToCanvas(this);
	
	// 立即恢复Slot设置
	if (NewSlot)
	{
		NewSlot->SetPosition(CurrentPosition);
		NewSlot->SetSize(CurrentSize);
		NewSlot->SetAnchors(CurrentAnchors);
		NewSlot->SetAlignment(CurrentAlignment);
		NewSlot->SetZOrder(CurrentZOrder);
	}

	// 强制Canvas重新布局
	Canvas->ForceLayoutPrepass();
	
	UE_LOG(LogTemp, Log, TEXT("[GaiaDraggableWindow] 窗口已置顶: %s (索引: %d -> %d)"), 
		*GetWindowTitle().ToString(), CurrentIndex, ChildCount - 1);
}

void UGaiaDraggableWindow::InitializeWindow()
{
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] InitializeWindow 调用"));

	// 设置标题栏
	if (TitleBar)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] 找到TitleBar组件"));
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] TitleBar类: %s"), *TitleBar->GetClass()->GetName());
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] TitleBar Visibility: %d"), (int32)TitleBar->GetVisibility());
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] TitleBar IsDraggable: %s"), TitleBar->IsDraggable() ? TEXT("true") : TEXT("false"));

		TitleBar->SetTitleText(DefaultWindowTitle);
		TitleBar->SetCloseButtonVisible(bShowCloseButton);

		// 绑定关闭事件
		TitleBar->OnCloseClicked.AddDynamic(this, &UGaiaDraggableWindow::HandleTitleBarCloseClicked);

		// 绑定拖动结束事件（用于保存位置）
		if (bSaveWindowPosition)
		{
			TitleBar->OnDragEnded.AddDynamic(this, &UGaiaDraggableWindow::HandleTitleBarDragEnded);
		}

		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] TitleBar初始化完成"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GaiaDraggableWindow] TitleBar组件不存在！"));
	}

	// 检查其他必需组件
	if (WindowBorder)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] 找到WindowBorder组件"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GaiaDraggableWindow] WindowBorder组件不存在！"));
	}

	if (ContentSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableWindow] 找到ContentSlot组件"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GaiaDraggableWindow] ContentSlot组件不存在！"));
	}

	// 应用窗口样式
	ApplyWindowStyle();
}

void UGaiaDraggableWindow::ApplyWindowStyle() const
{
	if (!WindowBorder)
	{
		return;
	}

	// 设置边框颜色
	FLinearColor BorderColor = WindowBorderColor;
	WindowBorder->SetBrushColor(BorderColor);

	// 设置边框宽度
	// 注意：需要在UMG中配置Border的Brush属性
	UE_LOG(LogTemp, Verbose, TEXT("[GaiaDraggableWindow] 应用窗口样式: Color=(%.2f, %.2f, %.2f, %.2f), Thickness=%.2f"),
		BorderColor.R, BorderColor.G, BorderColor.B, BorderColor.A, WindowBorderThickness);
}

void UGaiaDraggableWindow::LoadWindowPosition() const
{
	if (!bSaveWindowPosition || WindowPositionSaveKey.IsEmpty())
	{
		return;
	}

	// 这里可以从游戏配置或存档中加载位置
	// 示例实现（使用PlayerController的存档）
	if (APlayerController* PC = GetOwningPlayer())
	{
		// TODO: 实现位置加载逻辑
		// 例如从 GameUserSettings 或自定义存档系统加载

		UE_LOG(LogTemp, Verbose, TEXT("[GaiaDraggableWindow] 加载窗口位置: Key=%s"), *WindowPositionSaveKey);
	}
}

void UGaiaDraggableWindow::SaveWindowPosition() const
{
	if (!bSaveWindowPosition || WindowPositionSaveKey.IsEmpty() || !bIsShown)
	{
		return;
	}

	// 获取当前位置
	FVector2D CurrentPosition = GetCachedGeometry().GetAbsolutePosition();

	// 这里可以保存到游戏配置或存档
	// 示例实现（使用PlayerController的存档）
	if (APlayerController* PC = GetOwningPlayer())
	{
		// TODO: 实现位置保存逻辑
		// 例如保存到 GameUserSettings 或自定义存档系统

		UE_LOG(LogTemp, Verbose, TEXT("[GaiaDraggableWindow] 保存窗口位置: Key=%s, Position=(%.2f, %.2f)"),
			*WindowPositionSaveKey, CurrentPosition.X, CurrentPosition.Y);
	}
}

FVector2D UGaiaDraggableWindow::ClampWindowSize(const FVector2D& Size) const
{
	FVector2D ClampedSize = Size;

	// 限制最小大小
	ClampedSize.X = FMath::Max(ClampedSize.X, MinWindowSize.X);
	ClampedSize.Y = FMath::Max(ClampedSize.Y, MinWindowSize.Y);

	// 限制最大大小（如果设置了）
	if (MaxWindowSize.X > 0)
	{
		ClampedSize.X = FMath::Min(ClampedSize.X, MaxWindowSize.X);
	}
	if (MaxWindowSize.Y > 0)
	{
		ClampedSize.Y = FMath::Min(ClampedSize.Y, MaxWindowSize.Y);
	}

	return ClampedSize;
}

void UGaiaDraggableWindow::HandleTitleBarCloseClicked()
{
	// 关闭窗口
	CloseWindow();
}

void UGaiaDraggableWindow::HandleTitleBarDragEnded()
{
	// 拖动结束后保存位置
	if (bSaveWindowPosition)
	{
		SaveWindowPosition();
	}
}

