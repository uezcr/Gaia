# GaiaDraggableWindow 使用指南

## 概述

`UGaiaDraggableWindow` 是一个完整的可拖拽窗口基类，内置使用 `UGaiaDraggableTitleBar` 实现拖动功能。

## 核心特性

- ✅ **可拖拽标题栏**：使用 `GaiaDraggableTitleBar` 实现平滑拖动
- ✅ **自动大小和位置控制**：解决 `AddToViewport()` 的满屏问题
- ✅ **居中显示**：支持自动居中或指定位置
- ✅ **内容槽位**：使用 `NamedSlot` 实现内容自定义
- ✅ **关闭按钮**：内置关闭功能
- ✅ **大小限制**：支持最小/最大窗口大小
- ✅ **位置保存**：可选的窗口位置持久化
- ✅ **完全可配置**：窗口标题、大小、颜色等全部可配置

---

## 快速开始

### 1. 创建 Widget Blueprint

1. 在 Content Browser 中右键 → User Interface → Widget Blueprint
2. 父类选择 `GaiaDraggableWindow`
3. 命名为 `WBP_MyWindow`

### 2. 设计 UI 结构

在 UMG Designer 中，组件结构应该是（**这些组件会自动创建，因为使用了 BindWidget**）：

```
Canvas Panel (根)
  └─ WindowBorder (Border) ⭐ 必须命名为 "WindowBorder"
      └─ Vertical Box
          ├─ TitleBar (GaiaDraggableTitleBar) ⭐ 必须命名为 "TitleBar"
          └─ ContentSlot (Named Slot) ⭐ 必须命名为 "ContentSlot"
```

> **重要**：这三个组件必须使用精确的名称，否则 BindWidget 会失败。

### 3. 添加自定义内容

在 `ContentSlot` 中添加你的 UI 内容：

```
ContentSlot (Named Slot)
  └─ [你的内容]
      ├─ Text - 玩家名字
      ├─ Button - 确定
      └─ ...
```

### 4. 配置窗口属性

在 Blueprint 的 Details 面板中配置：

| 属性 | 说明 | 默认值 |
|------|------|--------|
| **Default Window Title** | 窗口标题 | "Window" |
| **Default Window Size** | 窗口大小 | (800, 600) |
| **Default Window Position** | 窗口位置（-1 表示居中） | (-1, -1) |
| **Auto Center On Show** | 是否自动居中 | true |
| **Min Window Size** | 最小窗口大小 | (400, 300) |
| **Max Window Size** | 最大窗口大小（0 表示不限制） | (0, 0) |
| **Show Close Button** | 是否显示关闭按钮 | true |
| **Window Border Color** | 窗口边框颜色 | 金色 |
| **Window Border Thickness** | 窗口边框宽度 | 2.0 |

### 5. 显示窗口

#### C++ 方式

```cpp
// 创建窗口
UGaiaDraggableWindow* MyWindow = CreateWidget<UGaiaDraggableWindow>(GetWorld(), MyWindowClass);

// 配置窗口（可选）
MyWindow->SetWindowTitle(NSLOCTEXT("UI", "MyTitle", "我的窗口"));
MyWindow->SetWindowSize(FVector2D(800, 600));

// 显示窗口（会自动处理大小和位置）
MyWindow->ShowWindow();
```

#### Blueprint 方式

```
Create Widget → WBP_MyWindow
  ↓
Show Window (调用此节点，不要用 Add To Viewport)
```

> **⚠️ 重要**：使用 `ShowWindow` 而不是 `AddToViewport`，这样才能正确设置窗口大小和位置。

---

## API 参考

### 窗口控制

#### ShowWindow
```cpp
UFUNCTION(BlueprintCallable, Category = "Window")
void ShowWindow();
```
显示窗口（添加到视口并设置位置）

**注意**：此函数会：
- 添加窗口到视口
- 设置对齐方式为左上角
- 设置窗口大小
- 设置窗口位置（居中或指定位置）
- 触发 `OnWindowShown` 事件

#### CloseWindow
```cpp
UFUNCTION(BlueprintCallable, Category = "Window")
void CloseWindow();
```
关闭窗口（从父级移除）

**注意**：此函数会：
- 保存窗口位置（如果启用）
- 从视口移除
- 触发 `OnWindowClosed` 事件

### 窗口属性

#### SetWindowTitle / GetWindowTitle
```cpp
UFUNCTION(BlueprintCallable, Category = "Window")
void SetWindowTitle(const FText& InTitle);

UFUNCTION(BlueprintPure, Category = "Window")
FText GetWindowTitle() const;
```
设置/获取窗口标题

#### SetWindowSize / GetWindowSize
```cpp
UFUNCTION(BlueprintCallable, Category = "Window")
void SetWindowSize(FVector2D InSize);

UFUNCTION(BlueprintPure, Category = "Window")
FVector2D GetWindowSize() const;
```
设置/获取窗口大小（会自动限制在最小/最大范围内）

#### SetWindowPosition / GetWindowPosition
```cpp
UFUNCTION(BlueprintCallable, Category = "Window")
void SetWindowPosition(FVector2D InPosition);

UFUNCTION(BlueprintPure, Category = "Window")
FVector2D GetWindowPosition() const;
```
设置/获取窗口位置

#### CenterWindow
```cpp
UFUNCTION(BlueprintCallable, Category = "Window")
void CenterWindow();
```
居中显示窗口

### 窗口行为

#### SetCloseButtonVisible
```cpp
UFUNCTION(BlueprintCallable, Category = "Window")
void SetCloseButtonVisible(bool bVisible);
```
设置是否显示关闭按钮

#### SetDraggable
```cpp
UFUNCTION(BlueprintCallable, Category = "Window")
void SetDraggable(bool bCanDrag);
```
设置是否可拖动

### 事件

#### OnWindowClosed
```cpp
UPROPERTY(BlueprintAssignable, Category = "Window|Events")
FOnWindowClosed OnWindowClosed;
```
窗口关闭时触发

#### OnWindowShown
```cpp
UPROPERTY(BlueprintAssignable, Category = "Window|Events")
FOnWindowShown OnWindowShown;
```
窗口显示时触发

---

## 常见问题

### Q1: 为什么 `AddToViewport()` 会导致窗口满屏？

**原因**：`AddToViewport()` 默认将 Widget 的 `Desired Size` 设置为视口大小，并且没有位置控制。

**解决方案**：使用 `ShowWindow()` 函数，它会：
```cpp
// 1. 设置对齐方式为左上角
SetAlignmentInViewport(FVector2D(0.0f, 0.0f));

// 2. 设置窗口大小
SetDesiredSizeInViewport(WindowSize);

// 3. 设置窗口位置
SetPositionInViewport(Position, false);
```

### Q2: 如何设置窗口的初始位置？

**方式1 - 居中显示（推荐）**：
```cpp
// 设置 Auto Center On Show = true（默认）
MyWindow->ShowWindow(); // 自动居中
```

**方式2 - 指定位置**：
```cpp
// 设置 Auto Center On Show = false
MyWindow->SetWindowPosition(FVector2D(100, 100));
MyWindow->ShowWindow();
```

**方式3 - Blueprint 配置**：
- 在 Details 面板中设置 `Default Window Position`
- 如果值为 `(-1, -1)` 则会居中
- 如果设置了具体坐标，则显示在该位置

### Q3: 窗口可以拖动出屏幕吗？

**不会**。`GaiaDraggableTitleBar` 已经实现了屏幕边界限制：
```cpp
// 在 GaiaDraggableTitleBar.cpp 中
NewPosition.X = FMath::Clamp(NewPosition.X, 0.0f, ViewportSize.X - WidgetSize.X);
NewPosition.Y = FMath::Clamp(NewPosition.Y, 0.0f, ViewportSize.Y - WidgetSize.Y);
```

### Q4: 如何保存窗口位置？

启用窗口位置保存功能：

```cpp
// C++
MyWindow->bSaveWindowPosition = true;
MyWindow->WindowPositionSaveKey = TEXT("MyWindow_Position");
```

或在 Blueprint 的 Details 面板中：
- `Save Window Position` = true
- `Window Position Save Key` = "MyWindow_Position"

**注意**：当前实现预留了接口，需要自己实现 `LoadWindowPosition()` 和 `SaveWindowPosition()` 逻辑（例如使用 `GameUserSettings` 或存档系统）。

### Q5: 如何自定义窗口样式？

**方式1 - 使用配置属性**：
```cpp
MyWindow->WindowBorderColor = FLinearColor::Red;
MyWindow->WindowBorderThickness = 3.0f;
```

**方式2 - 在 UMG 中修改 Border 样式**：
- 选择 `WindowBorder` 组件
- 修改 `Brush` 属性（颜色、图片、样式等）

**方式3 - 继承并重写 `ApplyWindowStyle()`**：
```cpp
void UMyCustomWindow::ApplyWindowStyle()
{
    Super::ApplyWindowStyle();
    
    // 自定义样式逻辑
    if (WindowBorder)
    {
        // 设置圆角
        // 设置阴影
        // 等等...
    }
}
```

---

## 完整示例

### C++ 创建窗口

```cpp
// MyPlayerController.h
UPROPERTY(EditDefaultsOnly, Category = "UI")
TSubclassOf<UGaiaDraggableWindow> InventoryWindowClass;

// MyPlayerController.cpp
void AMyPlayerController::ShowInventory()
{
    if (!InventoryWindow)
    {
        // 创建窗口
        InventoryWindow = CreateWidget<UGaiaDraggableWindow>(this, InventoryWindowClass);
        
        // 配置窗口
        InventoryWindow->SetWindowTitle(NSLOCTEXT("UI", "Inventory", "背包"));
        InventoryWindow->SetWindowSize(FVector2D(1000, 700));
        InventoryWindow->SetCloseButtonVisible(true);
        
        // 绑定关闭事件
        InventoryWindow->OnWindowClosed.AddDynamic(this, &AMyPlayerController::OnInventoryWindowClosed);
    }
    
    // 显示窗口
    InventoryWindow->ShowWindow();
}

void AMyPlayerController::OnInventoryWindowClosed()
{
    UE_LOG(LogTemp, Log, TEXT("背包窗口已关闭"));
    // 可以在这里做清理工作
}
```

### Blueprint 创建窗口

```
Event BeginPlay
  ↓
Create Widget (Class: WBP_InventoryWindow)
  ↓
Set Window Title (Text: "背包")
  ↓
Set Window Size (Size: X=1000, Y=700)
  ↓
Bind Event to OnWindowClosed → Print String "窗口已关闭"
  ↓
Show Window
```

---

## 设计亮点

### 1. 解决 `AddToViewport` 的满屏问题

通过 `ShowWindow()` 函数，使用以下 API：
- `SetAlignmentInViewport()` - 设置对齐方式
- `SetDesiredSizeInViewport()` - 设置期望大小
- `SetPositionInViewport()` - 设置位置

### 2. 组件自动绑定

使用 `meta = (BindWidget)` 确保 UMG 中的组件必须存在且名称正确：
```cpp
UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
TObjectPtr<UBorder> WindowBorder;
```

### 3. 内容自定义

使用 `NamedSlot` 实现内容区域的完全自定义：
```cpp
UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
TObjectPtr<UNamedSlot> ContentSlot;
```

### 4. 事件驱动

通过委托实现松耦合的事件通知：
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWindowClosed);
UPROPERTY(BlueprintAssignable)
FOnWindowClosed OnWindowClosed;
```

---

## 下一步

- [ ] 实现窗口位置保存/加载逻辑（连接到 GameUserSettings 或存档系统）
- [ ] 添加窗口最小化/最大化功能
- [ ] 添加窗口大小调整功能（拖动边缘）
- [ ] 添加多窗口 Z-Order 管理
- [ ] 添加窗口动画（打开/关闭）
- [ ] 添加窗口焦点管理

---

## 总结

`UGaiaDraggableWindow` 提供了一个完整的窗口化 UI 解决方案：

- ✅ **开箱即用**：创建 Blueprint 即可使用
- ✅ **解决痛点**：完美解决 `AddToViewport` 的满屏问题
- ✅ **高度可配置**：窗口大小、位置、样式全部可配置
- ✅ **易于扩展**：继承并重写即可自定义
- ✅ **Blueprint 友好**：所有功能都暴露给 Blueprint

现在你可以创建任意数量的可拖拽窗口，例如背包、角色面板、任务面板、设置面板等！

