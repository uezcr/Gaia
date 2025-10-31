// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaDraggableTitleBar.h"
#include "GaiaDraggableWindow.h"
#include "CommonTextBlock.h"
#include "CommonButtonBase.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"

UGaiaDraggableTitleBar::UGaiaDraggableTitleBar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), DragOffset()
{
	// 默认值
	bIsDraggable = true;
	bShowCloseButton = true;
	bIsDragging = false;
	DragOpacity = 0.7f;
	bConstrainToScreen = true;
	OriginalOpacity = 1.0f;
	DefaultTitleText = NSLOCTEXT("GaiaUI", "DefaultTitle", "Window");

	// 关键：设置为可见并接收鼠标输入
	//UUserWidget::SetVisibility(ESlateVisibility::Visible);
	// bIsFocusable 只能在构造时设置，且已经过时，不再需要手动设置

	//UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 构造函数调用，Visibility已设置为Visible"));
}

void UGaiaDraggableTitleBar::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] NativeConstruct 调用"));
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] bIsDraggable = %s"), bIsDraggable ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] Visibility = %d"), (int32)GetVisibility());

	// 设置默认标题
	if (TitleText && !DefaultTitleText.IsEmpty())
	{
		TitleText->SetText(DefaultTitleText);
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 标题文本已设置: %s"), *DefaultTitleText.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 标题文本组件不存在或标题为空"));
	}

	// 绑定关闭按钮
	if (CloseButton)
	{
		CloseButton->OnClicked().AddUObject(this, &UGaiaDraggableTitleBar::HandleCloseButtonClicked);
		CloseButton->SetVisibility(bShowCloseButton ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 关闭按钮已绑定"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 关闭按钮不存在"));
	}

	// 保存原始透明度
	if (UUserWidget* ParentWidget = GetParentWidget())
	{
		OriginalOpacity = ParentWidget->GetRenderOpacity();
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 找到父Widget: %s, 透明度: %.2f"), 
			*ParentWidget->GetClass()->GetName(), OriginalOpacity);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 未找到父Widget"));
	}
}

void UGaiaDraggableTitleBar::NativeDestruct()
{
	// 清理
	if (CloseButton)
	{
		CloseButton->OnClicked().RemoveAll(this);
	}

	Super::NativeDestruct();
}

FReply UGaiaDraggableTitleBar::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] NativeOnMouseButtonDown 调用"));
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 按键: %s"), *InMouseEvent.GetEffectingButton().ToString());
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] bIsDraggable: %s"), bIsDraggable ? TEXT("true") : TEXT("false"));

	// 任何左键按下都应该置顶窗口
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 获取父窗口并调用其BringToFront方法
		if (UGaiaDraggableWindow* ParentWindow = Cast<UGaiaDraggableWindow>(GetParentWidget()))
		{
			ParentWindow->BringToFront();
			UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 窗口已置顶（调用Window的BringToFront）"));
		}
		else
		{
			// 如果父窗口不是GaiaDraggableWindow，则使用原有的BringWindowToFront方法
			UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 窗口已置顶（使用TitleBar的BringWindowToFront）"));
		}
	}

	// 只有左键且可拖动时才开始拖动
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDraggable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 开始拖动检测"));
		StartDragging(InGeometry, InMouseEvent);
		return FReply::Handled().CaptureMouse(TakeWidget()).PreventThrottling();
	}

	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 未处理鼠标按下（不是左键或不可拖动）"));
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UGaiaDraggableTitleBar::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		EndDragging();
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UGaiaDraggableTitleBar::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging)
	{
		UpdateDragging(InGeometry, InMouseEvent);
		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UGaiaDraggableTitleBar::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		// 触发双击事件
		OnTitleBarDoubleClicked.Broadcast();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UGaiaDraggableTitleBar::SetTitleText(const FText& InTitleText)
{
	DefaultTitleText = InTitleText;
	
	if (TitleText)
	{
		TitleText->SetText(InTitleText);
	}
}

FText UGaiaDraggableTitleBar::GetTitleText() const
{
	if (TitleText)
	{
		return TitleText->GetText();
	}
	
	return DefaultTitleText;
}

void UGaiaDraggableTitleBar::SetCloseButtonVisible(bool bVisible)
{
	bShowCloseButton = bVisible;
	
	if (CloseButton)
	{
		CloseButton->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UGaiaDraggableTitleBar::HandleCloseButtonClicked()
{
	// 触发关闭事件
	OnCloseClicked.Broadcast();
}

void UGaiaDraggableTitleBar::StartDragging(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] StartDragging 调用"));
	
	bIsDragging = true;

	UUserWidget* ParentWidget = GetParentWidget();
	if (!ParentWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[GaiaDraggableTitleBar] 无法获取父Widget！"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 父Widget: %s"), *ParentWidget->GetClass()->GetName());

	// 计算拖动偏移量（相对于标题栏的点击位置）
	FVector2D MousePositionInWidget = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	DragOffset = MousePositionInWidget;

	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 标题栏内点击位置: (%.2f, %.2f)"), MousePositionInWidget.X, MousePositionInWidget.Y);
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 拖动偏移: (%.2f, %.2f)"), DragOffset.X, DragOffset.Y);

	// 设置拖动时的透明度
	if (DragOpacity < 1.0f)
	{
		ParentWidget->SetRenderOpacity(DragOpacity);
	}

	// 触发拖动开始事件
	OnDragStarted.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 开始拖动完成"));
}

void UGaiaDraggableTitleBar::UpdateDragging(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) const
{
	if (!bIsDragging)
	{
		return;
	}

	UUserWidget* ParentWidget = GetParentWidget();
	if (!ParentWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[GaiaDraggableTitleBar] UpdateDragging: 无法获取父Widget！"));
		return;
	}

	// 获取父Widget的Canvas Slot
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ParentWidget->Slot);
	if (!CanvasSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("[GaiaDraggableTitleBar] 父Widget不在Canvas中！请确保窗口添加到Canvas Panel中。"));
		return;
	}

	// 计算新位置（鼠标位置 - 标题栏内的点击偏移 - 标题栏在窗口内的位置）
	FVector2D MousePosition = InMouseEvent.GetScreenSpacePosition();
	
	// 获取TitleBar在父Widget中的位置
	FVector2D TitleBarPositionInParent = GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D::ZeroVector);
	
	// 计算窗口的新位置
	FVector2D NewPosition = MousePosition - DragOffset - TitleBarPositionInParent;

	// 获取Canvas Panel (父容器)
	UCanvasPanel* Canvas = Cast<UCanvasPanel>(CanvasSlot->Parent);
	if (Canvas)
	{
		// 转换为Canvas的本地坐标
		FVector2D CanvasLocalPosition = Canvas->GetCachedGeometry().AbsoluteToLocal(MousePosition);
		NewPosition = CanvasLocalPosition - DragOffset - TitleBarPositionInParent;

		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 拖动更新:"));
		UE_LOG(LogTemp, Warning, TEXT("  鼠标屏幕位置: (%.2f, %.2f)"), MousePosition.X, MousePosition.Y);
		UE_LOG(LogTemp, Warning, TEXT("  Canvas本地位置: (%.2f, %.2f)"), CanvasLocalPosition.X, CanvasLocalPosition.Y);
		UE_LOG(LogTemp, Warning, TEXT("  拖动偏移: (%.2f, %.2f)"), DragOffset.X, DragOffset.Y);
		UE_LOG(LogTemp, Warning, TEXT("  标题栏在窗口内位置: (%.2f, %.2f)"), TitleBarPositionInParent.X, TitleBarPositionInParent.Y);
		UE_LOG(LogTemp, Warning, TEXT("  窗口新位置: (%.2f, %.2f)"), NewPosition.X, NewPosition.Y);

		// 限制在Canvas内
		if (bConstrainToScreen)
		{
			FVector2D WidgetSize = ParentWidget->GetCachedGeometry().GetLocalSize();
			FVector2D CanvasSize = Canvas->GetCachedGeometry().GetLocalSize();
			
			NewPosition.X = FMath::Clamp(NewPosition.X, 0.0f, CanvasSize.X - WidgetSize.X);
			NewPosition.Y = FMath::Clamp(NewPosition.Y, 0.0f, CanvasSize.Y - WidgetSize.Y);
			
			UE_LOG(LogTemp, Warning, TEXT("  限制后位置: (%.2f, %.2f)"), NewPosition.X, NewPosition.Y);
		}
	}

	// 更新Canvas Slot位置
	CanvasSlot->SetPosition(NewPosition);
}

void UGaiaDraggableTitleBar::EndDragging()
{
	if (!bIsDragging)
	{
		return;
	}

	bIsDragging = false;

	// 恢复原始透明度
	UUserWidget* ParentWidget = GetParentWidget();
	if (ParentWidget && DragOpacity < 1.0f)
	{
		ParentWidget->SetRenderOpacity(OriginalOpacity);
	}

	// 触发拖动结束事件
	OnDragEnded.Broadcast();

	UE_LOG(LogTemp, Verbose, TEXT("[GaiaDraggableTitleBar] 结束拖动"));
}

UUserWidget* UGaiaDraggableTitleBar::GetParentWidget() const
{
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] GetParentWidget 开始查找"));

	// 方法1: 向上查找父Widget
	UWidget* Parent = GetParent();
	int32 Level = 0;
	while (Parent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 层级 %d: %s"), Level, *Parent->GetClass()->GetName());
		
		if (UUserWidget* ParentUserWidget = Cast<UUserWidget>(Parent))
		{
			UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 找到父Widget: %s"), *ParentUserWidget->GetClass()->GetName());
			return ParentUserWidget;
		}
		Parent = Parent->GetParent();
		Level++;
	}

	// 方法2: 尝试从 WidgetTree 查找
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 方法1失败，尝试从 WidgetTree 查找"));
	if (UWidgetTree* Tree = Cast<UWidgetTree>(GetOuter()))
	{
		if (UUserWidget* RootWidget = Cast<UUserWidget>(Tree->GetOuter()))
		{
			UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 从 WidgetTree 找到父Widget: %s"), *RootWidget->GetClass()->GetName());
			return RootWidget;
		}
	}

	// 方法3: 尝试从 Outer 查找
	UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 方法2失败，尝试从 Outer 查找"));
	UObject* Outer = GetOuter();
	while (Outer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] Outer: %s"), *Outer->GetClass()->GetName());
		
		if (UUserWidget* OuterWidget = Cast<UUserWidget>(Outer))
		{
			UE_LOG(LogTemp, Warning, TEXT("[GaiaDraggableTitleBar] 从 Outer 找到父Widget: %s"), *OuterWidget->GetClass()->GetName());
			return OuterWidget;
		}
		Outer = Outer->GetOuter();
	}

	UE_LOG(LogTemp, Error, TEXT("[GaiaDraggableTitleBar] 所有方法都失败，无法找到父Widget"));
	return nullptr;
}

FVector2D UGaiaDraggableTitleBar::ConstrainPositionToScreen(const FVector2D& Position, const FVector2D& Size)
{
	FVector2D ConstrainedPosition = Position;

	// 获取视口大小
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	else
	{
		// 如果无法获取视口，使用默认值
		ViewportSize = FVector2D(1920, 1080);
	}

	// 限制左边
	if (ConstrainedPosition.X < 0)
	{
		ConstrainedPosition.X = 0;
	}

	// 限制上边
	if (ConstrainedPosition.Y < 0)
	{
		ConstrainedPosition.Y = 0;
	}

	// 限制右边（确保至少有一部分可见）
	const float MinVisibleWidth = FMath::Min(Size.X, 100.0f);
	if (ConstrainedPosition.X + MinVisibleWidth > ViewportSize.X)
	{
		ConstrainedPosition.X = ViewportSize.X - MinVisibleWidth;
	}

	// 限制下边（确保至少有一部分可见）
	const float MinVisibleHeight = FMath::Min(Size.Y, 50.0f);
	if (ConstrainedPosition.Y + MinVisibleHeight > ViewportSize.Y)
	{
		ConstrainedPosition.Y = ViewportSize.Y - MinVisibleHeight;
	}

	return ConstrainedPosition;
}

