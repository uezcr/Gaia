# Canvas 窗口系统使用指南

## 概述

重新设计的窗口系统基于 **Canvas Panel** 管理所有窗口，提供更精确的位置控制和更简单的拖动逻辑。

## 核心架构

```
主视口UI (UUserWidget)
  └─ Canvas Panel (UCanvasPanel)
      ├─ Window1 (UGaiaDraggableWindow)
      ├─ Window2 (UGaiaDraggableWindow)
      └─ Window3 (UGaiaDraggableWindow)
```

## 关键改进

### 1. 基于 Canvas 的窗口管理

- ✅ 所有窗口都添加到同一个 Canvas 中
- ✅ 使用 Canvas Slot 进行位置控制
- ✅ 拖动时直接操作 Slot 位置，而不是 `SetPositionInViewport`
- ✅ 更精确的鼠标偏移计算

### 2. 简化的拖动逻辑

**旧方式**（复杂且容易出错）：
```cpp
// 计算窗口绝对位置的偏移
DragOffset = MousePosition - WidgetPosition;
NewPosition = MousePosition - DragOffset;
SetPositionInViewport(NewPosition);
```

**新方式**（简单且精确）：
```cpp
// 计算在标题栏内的点击位置
DragOffset = TitleBar.AbsoluteToLocal(MousePosition);

// 转换为Canvas本地坐标
CanvasLocalPosition = Canvas.AbsoluteToLocal(MousePosition);
NewPosition = CanvasLocalPosition - DragOffset - TitleBarPositionInParent;

// 直接设置Slot位置
CanvasSlot->SetPosition(NewPosition);
```

### 3. 自动屏幕限制

窗口自动限制在 Canvas 边界内：
```cpp
NewPosition.X = FMath::Clamp(NewPosition.X, 0.0f, CanvasSize.X - WidgetSize.X);
NewPosition.Y = FMath::Clamp(NewPosition.Y, 0.0f, CanvasSize.Y - WidgetSize.Y);
```

---

## 使用方法

### 步骤 1: 创建主视口 UI

创建一个主 UI Widget（例如 `WBP_MainUI`）：

```
Canvas Panel (Fill Screen)
  └─ (将在运行时添加窗口)
```

**Blueprint 示例**：
1. 创建 Widget Blueprint `WBP_MainUI`
2. 添加 Canvas Panel，设置为 Fill Screen
3. 命名为 `WindowCanvas`

### 步骤 2: 创建窗口 Blueprint

创建窗口 Blueprint（例如 `WBP_InventoryWindow`）：

1. 父类选择 `GaiaDraggableWindow`
2. 在 UMG Designer 中添加必需组件：
   - `WindowBorder` (Border)
   - `TitleBar` (GaiaDraggableTitleBar)
   - `ContentSlot` (Named Slot)
3. 在 `ContentSlot` 中添加你的内容

### 步骤 3: 显示窗口

#### C++ 方式

```cpp
// 1. 创建主UI并添加到视口
UMainUI* MainUI = CreateWidget<UMainUI>(GetWorld(), MainUIClass);
MainUI->AddToViewport();

// 2. 获取Canvas
UCanvasPanel* WindowCanvas = MainUI->GetWindowCanvas();

// 3. 创建窗口
UGaiaDraggableWindow* InventoryWindow = CreateWidget<UGaiaDraggableWindow>(GetWorld(), InventoryWindowClass);

// 4. 配置窗口
InventoryWindow->SetWindowTitle(NSLOCTEXT("UI", "Inventory", "背包"));
InventoryWindow->SetWindowSize(FVector2D(1000, 700));

// 5. 显示窗口（添加到Canvas）
InventoryWindow->ShowInCanvas(WindowCanvas);  // 居中显示
// 或指定位置
// InventoryWindow->ShowInCanvas(WindowCanvas, FVector2D(100, 100));
```

#### Blueprint 方式

```
Event BeginPlay
  ↓
Create Widget (WBP_MainUI)
  ↓
Add To Viewport
  ↓
Get Window Canvas (从 MainUI)
  ↓
Create Widget (WBP_InventoryWindow)
  ↓
Set Window Title → "背包"
  ↓
Set Window Size → (X=1000, Y=700)
  ↓
Show In Canvas (Canvas = Window Canvas, Position = -1,-1)
```

---

## API 参考

### UGaiaDraggableWindow

#### ShowInCanvas

```cpp
/**
 * 显示窗口（添加到指定Canvas并设置位置）
 * @param Canvas - 目标Canvas Panel
 * @param Position - 窗口位置（-1表示居中）
 */
UFUNCTION(BlueprintCallable, Category = "Window")
void ShowInCanvas(UCanvasPanel* Canvas, FVector2D Position = FVector2D(-1, -1));
```

**示例**：
```cpp
// 居中显示
Window->ShowInCanvas(Canvas);

// 指定位置
Window->ShowInCanvas(Canvas, FVector2D(100, 100));

// 如果设置了 bAutoCenterOnShow = true，会忽略Position并居中
```

#### ShowWindow（已过时）

```cpp
/**
 * 显示窗口（添加到视口并设置位置） - 已过时
 * @deprecated 请使用 ShowInCanvas 将窗口添加到Canvas中
 */
UFUNCTION(BlueprintCallable, Category = "Window", meta = (DeprecatedFunction))
void ShowWindow();
```

---

## 拖动原理详解

### 坐标系统

```
屏幕坐标 (Absolute)
  ↓ Canvas.AbsoluteToLocal()
Canvas本地坐标
  ↓ 减去偏移
窗口位置
```

### 拖动流程

#### 1. 开始拖动 (StartDragging)

```cpp
// 计算鼠标在标题栏内的点击位置
FVector2D MousePositionInTitleBar = TitleBar.AbsoluteToLocal(MouseScreenPosition);
DragOffset = MousePositionInTitleBar;  // 保存偏移量
```

**例如**：
- 鼠标屏幕坐标：`(500, 200)`
- 标题栏绝对位置：`(400, 180)`
- 标题栏内点击位置：`(100, 20)` ← 这就是 `DragOffset`

#### 2. 更新拖动 (UpdateDragging)

```cpp
// 获取Canvas Slot
UCanvasPanelSlot* CanvasSlot = ParentWidget->Slot;

// 转换为Canvas本地坐标
FVector2D CanvasLocalPosition = Canvas->AbsoluteToLocal(MouseScreenPosition);

// 计算窗口新位置
FVector2D NewPosition = CanvasLocalPosition - DragOffset - TitleBarPositionInParent;

// 限制在Canvas内
NewPosition.X = FMath::Clamp(NewPosition.X, 0.0f, CanvasSize.X - WidgetSize.X);
NewPosition.Y = FMath::Clamp(NewPosition.Y, 0.0f, CanvasSize.Y - WidgetSize.Y);

// 设置位置
CanvasSlot->SetPosition(NewPosition);
```

**例如**：
- 鼠标移动到屏幕坐标：`(600, 300)`
- Canvas本地坐标：`(600, 300)`（假设Canvas也在原点）
- DragOffset：`(100, 20)`
- TitleBarPositionInParent：`(0, 0)`（假设标题栏在窗口顶部）
- 窗口新位置：`(600 - 100 - 0, 300 - 20 - 0) = (500, 280)`

#### 3. 结束拖动 (EndDragging)

```cpp
bIsDragging = false;
// 恢复透明度
// 触发事件
```

---

## 调试日志

拖动时会输出详细日志：

```
[GaiaDraggableTitleBar] StartDragging 调用
[GaiaDraggableTitleBar] 标题栏内点击位置: (100.00, 20.00)
[GaiaDraggableTitleBar] 拖动偏移: (100.00, 20.00)
[GaiaDraggableTitleBar] 开始拖动完成

[GaiaDraggableTitleBar] 拖动更新:
  鼠标屏幕位置: (600.00, 300.00)
  Canvas本地位置: (600.00, 300.00)
  拖动偏移: (100.00, 20.00)
  标题栏在窗口内位置: (0.00, 0.00)
  窗口新位置: (500.00, 280.00)
```

**如果看到错误**：
```
[GaiaDraggableTitleBar] 父Widget不在Canvas中！请确保窗口添加到Canvas Panel中。
```
→ 请使用 `ShowInCanvas` 而不是 `ShowWindow` 或 `AddToViewport`

---

## 完整示例

### 主UI (C++)

```cpp
// MainUI.h
UCLASS()
class UMainUI : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    // Canvas Panel用于管理所有窗口
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCanvasPanel> WindowCanvas;

    // 显示窗口的辅助函数
    UFUNCTION(BlueprintCallable)
    void ShowInventoryWindow();

private:
    UPROPERTY()
    TObjectPtr<UGaiaDraggableWindow> InventoryWindow;
};

// MainUI.cpp
void UMainUI::ShowInventoryWindow()
{
    if (!InventoryWindow)
    {
        InventoryWindow = CreateWidget<UGaiaDraggableWindow>(GetWorld(), InventoryWindowClass);
        InventoryWindow->SetWindowTitle(NSLOCTEXT("UI", "Inventory", "背包"));
        InventoryWindow->SetWindowSize(FVector2D(1000, 700));
    }

    InventoryWindow->ShowInCanvas(WindowCanvas);
}
```

### PlayerController

```cpp
void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 创建主UI
    MainUI = CreateWidget<UMainUI>(this, MainUIClass);
    MainUI->AddToViewport();
}

void AMyPlayerController::ToggleInventory()
{
    if (MainUI)
    {
        MainUI->ShowInventoryWindow();
    }
}
```

---

## 常见问题

### Q1: 窗口拖动时跳动或不跟随鼠标？

**原因**：窗口没有添加到Canvas中，而是直接 `AddToViewport`。

**解决方案**：
```cpp
// ❌ 错误
Window->ShowWindow();  // 已过时
Window->AddToViewport();  // 不要直接使用

// ✅ 正确
Window->ShowInCanvas(Canvas);
```

### Q2: 如何获取Canvas？

**方案1 - 从主UI获取**：
```cpp
UMainUI* MainUI = GetMainUI();  // 你的获取主UI的方法
UCanvasPanel* Canvas = MainUI->GetWindowCanvas();
```

**方案2 - BindWidget**：
```cpp
// 在主UI类中
UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
TObjectPtr<UCanvasPanel> WindowCanvas;
```

### Q3: 如何限制窗口在Canvas外？

窗口会**自动限制在Canvas内**（如果 `bConstrainToScreen = true`）。

如果需要自定义限制，可以重写：
```cpp
void MyWindow::SetWindowPosition(FVector2D InPosition)
{
    // 自定义限制逻辑
    InPosition.X = FMath::Max(MinX, FMath::Min(InPosition.X, MaxX));
    InPosition.Y = FMath::Max(MinY, FMath::Min(InPosition.Y, MaxY));
    
    Super::SetWindowPosition(InPosition);
}
```

### Q4: 如何实现多窗口Z-Order管理？

Canvas 自动管理 Z-Order（后添加的在上层）。

如果需要手动调整：
```cpp
// 将窗口置顶
Canvas->ShiftChild(WindowIndex, Canvas->GetChildrenCount() - 1);
```

---

## 性能优化

### 1. 对象池

避免频繁创建/销毁窗口：
```cpp
// 创建窗口池
TArray<UGaiaDraggableWindow*> WindowPool;

UGaiaDraggableWindow* GetOrCreateWindow()
{
    // 查找隐藏的窗口
    for (auto* Window : WindowPool)
    {
        if (!Window->IsVisible())
        {
            return Window;
        }
    }
    
    // 创建新窗口
    auto* NewWindow = CreateWidget<UGaiaDraggableWindow>(...);
    WindowPool.Add(NewWindow);
    return NewWindow;
}
```

### 2. 延迟加载

只在需要时创建窗口：
```cpp
void ShowWindow()
{
    if (!Window)
    {
        Window = CreateWidget<UGaiaDraggableWindow>(...);
    }
    Window->ShowInCanvas(Canvas);
}
```

---

## 总结

### 新架构的优势

1. ✅ **更简单的拖动逻辑**：直接操作Canvas Slot
2. ✅ **更精确的位置控制**：使用Canvas本地坐标
3. ✅ **自动屏幕限制**：窗口不会拖出Canvas
4. ✅ **更好的性能**：减少坐标转换次数
5. ✅ **更清晰的架构**：所有窗口统一管理

### 使用原则

- ✅ 始终使用 `ShowInCanvas` 而不是 `ShowWindow`
- ✅ 在主UI中创建一个Canvas Panel
- ✅ 所有窗口都添加到同一个Canvas
- ✅ 不要直接使用 `AddToViewport` 添加窗口

现在你可以创建完美跟随鼠标的可拖拽窗口了！🎉

