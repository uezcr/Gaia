# CommonUI窗口化库存系统设计

## 📋 设计目标

创建一个基于CommonUI的现代化、窗口化的库存UI系统，支持：
- ✅ 多窗口管理（可同时打开多个容器）
- ✅ 窗口拖动、缩放、最小化
- ✅ 层级管理（Z-Order）
- ✅ 窗口对齐和吸附
- ✅ 完整的拖放系统
- ✅ 右键菜单
- ✅ 物品Tooltip
- ✅ 网络同步

## 🏗️ 系统架构

### 核心组件层次

```
UGaiaInventoryUIManager (管理器)
    ↓
UGaiaPrimaryGameLayout (主布局)
    ↓
CommonActivatableWidgetStack (Layer系统)
    ├─ GameLayer (HUD、快捷栏)
    ├─ WindowLayer (容器窗口) ⭐ 窗口化
    ├─ MenuLayer (右键菜单)
    └─ ModalLayer (对话框)
        ↓
UGaiaWindowWidget (窗口基类) ⭐ 新增
    ├─ UGaiaContainerWindowWidget (容器窗口)
    ├─ UGaiaCharacterWindowWidget (角色窗口)
    └─ UGaiaCraftingWindowWidget (制作窗口)
```

### 窗口化设计

#### 1. 窗口基类（UGaiaWindowWidget）
```cpp
// 继承自 CommonActivatableWidget，支持激活/停用
class UGaiaWindowWidget : public UCommonActivatableWidget
{
    // 窗口特性
    - 标题栏（可拖动）
    - 关闭按钮
    - 最小化按钮
    - 窗口内容区域
    - 窗口大小（可配置）
    - 窗口位置（可保存）
    - Z-Order管理
    
    // 功能
    - 拖动窗口
    - 最小化/恢复
    - 关闭窗口
    - 聚焦窗口（带到最前面）
    - 保存/加载窗口位置
};
```

#### 2. 容器窗口（UGaiaContainerWindowWidget）
```cpp
class UGaiaContainerWindowWidget : public UGaiaWindowWidget
{
    // 容器特定功能
    - 容器网格显示
    - 容器标题（动态）
    - 容器图标
    - 容器信息（体积、槽位数）
    
    // 内容
    - UGaiaContainerGridWidget (网格)
    - 调试信息面板（可选）
};
```

#### 3. 窗口管理器扩展
```cpp
class UGaiaInventoryUIManager
{
    // 窗口管理
    - TArray<UGaiaWindowWidget*> OpenWindows;
    - UGaiaWindowWidget* FocusedWindow;
    
    // 功能
    - 打开窗口
    - 关闭窗口
    - 关闭所有窗口
    - 聚焦窗口
    - 排列窗口（级联、平铺）
    - 保存窗口布局
    - 加载窗口布局
};
```

## 🎨 UI组件设计

### 1. 窗口标题栏（Window Title Bar）

**组件**：
- `Border` (背景)
  - `HorizontalBox`
    - `Image` (窗口图标)
    - `TextBlock` (标题)
    - `Spacer`
    - `Button` (最小化)
    - `Button` (关闭)

**功能**：
- 左键拖动：移动窗口
- 双击：最大化/恢复
- 右键：窗口菜单（最小化、关闭、置顶等）

### 2. 窗口内容区（Window Content）

**组件**：
- `Border` (边框)
  - `VerticalBox`
    - `ScrollBox` (内容滚动)
      - 实际内容Widget

**功能**：
- 自动滚动
- 内容适配

### 3. 容器网格（Container Grid）

**组件**：
- `UniformGridPanel` (均匀网格)
  - `UGaiaItemSlotWidget` × N (槽位)

**功能**：
- 动态槽位数量
- 自动布局
- 拖放支持

### 4. 物品槽位（Item Slot）

**组件**：
- `Border` (槽位背景)
  - `Overlay`
    - `Image` (物品图标)
    - `TextBlock` (数量)
    - `Border` (高亮边框)

**功能**：
- 拖放操作
- 右键菜单
- 悬停Tooltip
- 视觉反馈

## 🎯 窗口化特性

### 1. 窗口拖动
```cpp
// 标题栏拖动
void OnTitleBarMouseButtonDown(FGeometry, FPointerEvent)
{
    bIsDragging = true;
    DragOffset = MousePosition - WindowPosition;
}

void OnTitleBarMouseMove(FGeometry, FPointerEvent)
{
    if (bIsDragging)
    {
        WindowPosition = MousePosition - DragOffset;
        SetRenderTranslation(WindowPosition);
    }
}
```

### 2. 窗口聚焦（Z-Order）
```cpp
void OnWindowClicked()
{
    UIManager->BringWindowToFront(this);
    UIManager->SetFocusedWindow(this);
}

void BringWindowToFront(UGaiaWindowWidget* Window)
{
    // 重新排列ZOrder
    int32 MaxZOrder = 0;
    for (UGaiaWindowWidget* Win : OpenWindows)
    {
        if (Win != Window)
        {
            Win->SetZOrder(Win->GetZOrder());
        }
        MaxZOrder = FMath::Max(MaxZOrder, Win->GetZOrder());
    }
    Window->SetZOrder(MaxZOrder + 1);
}
```

### 3. 窗口最小化
```cpp
void MinimizeWindow()
{
    bIsMinimized = true;
    ContentPanel->SetVisibility(ESlateVisibility::Collapsed);
    TitleBar->SetVisibility(ESlateVisibility::Visible);
    
    // 保存原始大小
    SavedSize = GetDesiredSize();
    SetDesiredSize(FVector2D(MinimizedWidth, TitleBarHeight));
}

void RestoreWindow()
{
    bIsMinimized = false;
    ContentPanel->SetVisibility(ESlateVisibility::Visible);
    SetDesiredSize(SavedSize);
}
```

### 4. 窗口吸附
```cpp
void CheckWindowSnapping()
{
    const float SnapDistance = 10.0f;
    FVector2D ViewportSize = GetViewportSize();
    
    // 检查边缘吸附
    if (FMath::Abs(WindowPosition.X) < SnapDistance)
        WindowPosition.X = 0; // 左边缘
        
    if (FMath::Abs(WindowPosition.X + WindowSize.X - ViewportSize.X) < SnapDistance)
        WindowPosition.X = ViewportSize.X - WindowSize.X; // 右边缘
        
    // 检查其他窗口吸附
    for (UGaiaWindowWidget* OtherWindow : OpenWindows)
    {
        if (OtherWindow == this) continue;
        
        FVector2D OtherPos = OtherWindow->GetPosition();
        FVector2D OtherSize = OtherWindow->GetSize();
        
        // 检查左右吸附
        if (FMath::Abs((WindowPosition.X + WindowSize.X) - OtherPos.X) < SnapDistance)
        {
            WindowPosition.X = OtherPos.X - WindowSize.X;
        }
    }
}
```

## 🎨 视觉设计

### 窗口样式

#### 默认样式
```css
标题栏：
- 背景：半透明深灰色 (0, 0, 0, 0.8)
- 高度：32px
- 字体：16px, 白色
- 图标：24x24

窗口边框：
- 颜色：金色 (255, 215, 0, 1.0)
- 宽度：2px
- 圆角：4px

内容区域：
- 背景：半透明黑色 (0, 0, 0, 0.85)
- 内边距：8px

槽位：
- 大小：64x64
- 间距：4px
- 背景：深灰色 (40, 40, 40, 1.0)
```

#### 聚焦样式
```css
标题栏：
- 背景：蓝色高亮 (30, 144, 255, 0.9)

窗口边框：
- 颜色：亮金色 (255, 223, 0, 1.0)
- 阴影：外发光效果
```

#### 悬停样式
```css
按钮：
- 背景：亮度+20%
- 边框：白色高亮

槽位：
- 边框：黄色高亮 (255, 255, 0, 1.0)
- 阴影：内发光效果
```

## 🔧 技术实现

### 1. 窗口Widget基类

**文件**：`GaiaWindowWidget.h/.cpp`

```cpp
UCLASS(Abstract, Blueprintable)
class GAIAGAME_API UGaiaWindowWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    // 窗口属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window")
    FText WindowTitle;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window")
    FVector2D DefaultSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window")
    bool bCanMinimize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Window")
    bool bCanResize;
    
    // 窗口组件（在UMG中绑定）
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UBorder* TitleBar;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* TitleText;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* MinimizeButton;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* CloseButton;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UBorder* ContentBorder;
    
    // 窗口状态
    UPROPERTY(BlueprintReadOnly, Category = "Window")
    bool bIsMinimized;
    
    UPROPERTY(BlueprintReadOnly, Category = "Window")
    bool bIsDragging;
    
    UPROPERTY(BlueprintReadOnly, Category = "Window")
    int32 ZOrder;
    
    // 功能函数
    UFUNCTION(BlueprintCallable, Category = "Window")
    void MinimizeWindow();
    
    UFUNCTION(BlueprintCallable, Category = "Window")
    void RestoreWindow();
    
    UFUNCTION(BlueprintCallable, Category = "Window")
    void CloseWindow();
    
    UFUNCTION(BlueprintCallable, Category = "Window")
    void BringToFront();
    
    UFUNCTION(BlueprintCallable, Category = "Window")
    void SetWindowPosition(FVector2D Position);
    
    UFUNCTION(BlueprintCallable, Category = "Window")
    FVector2D GetWindowPosition() const;
    
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    // 事件处理
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
private:
    FVector2D DragOffset;
    FVector2D SavedSize;
    
    UFUNCTION()
    void OnMinimizeButtonClicked();
    
    UFUNCTION()
    void OnCloseButtonClicked();
};
```

### 2. 容器窗口Widget

**文件**：`GaiaContainerWindowWidget.h/.cpp`

```cpp
UCLASS(Blueprintable)
class GAIAGAME_API UGaiaContainerWindowWidget : public UGaiaWindowWidget
{
    GENERATED_BODY()

public:
    // 容器网格
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UGaiaContainerGridWidget* ContainerGrid;
    
    // 容器信息
    UPROPERTY(BlueprintReadOnly, Category = "Container")
    FGuid ContainerUID;
    
    // 初始化
    UFUNCTION(BlueprintCallable, Category = "Container")
    void InitializeForContainer(const FGuid& InContainerUID);
    
    // 更新
    UFUNCTION(BlueprintCallable, Category = "Container")
    void RefreshContainer();
    
protected:
    virtual void NativeConstruct() override;
    
private:
    void UpdateWindowTitle();
    void SubscribeToContainerEvents();
    void UnsubscribeFromContainerEvents();
    
    // RPC回调
    UFUNCTION()
    void OnContainerDataReceived(const FGaiaContainerInstance& Container);
    
    UFUNCTION()
    void OnContainerUpdated(const FGuid& UpdatedContainerUID);
};
```

### 3. UI管理器扩展

**文件**：`GaiaInventoryUIManager.h/.cpp`

```cpp
UCLASS()
class GAIAGAME_API UGaiaInventoryUIManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // 窗口管理
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    UGaiaContainerWindowWidget* OpenContainerWindow(const FGuid& ContainerUID);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void CloseContainerWindow(UGaiaContainerWindowWidget* Window);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void CloseAllContainerWindows();
    
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    bool IsContainerWindowOpen(const FGuid& ContainerUID) const;
    
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void BringWindowToFront(UGaiaWindowWidget* Window);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void ArrangeWindowsCascade();
    
    UFUNCTION(BlueprintCallable, Category = "Inventory UI")
    void ArrangeWindowsTile();
    
    // 窗口类配置
    UPROPERTY(EditDefaultsOnly, Category = "UI Classes")
    TSubclassOf<UGaiaContainerWindowWidget> ContainerWindowClass;
    
private:
    // 打开的窗口列表
    UPROPERTY()
    TArray<UGaiaWindowWidget*> OpenWindows;
    
    // 当前聚焦的窗口
    UPROPERTY()
    UGaiaWindowWidget* FocusedWindow;
    
    // 窗口层
    UCommonActivatableWidgetStack* WindowLayer;
};
```

## 📋 实现步骤

### 第1阶段：基础窗口系统（2-3小时）
1. ✅ 创建 `UGaiaWindowWidget` 基类
2. ✅ 实现窗口拖动
3. ✅ 实现窗口关闭
4. ✅ 实现Z-Order管理
5. ✅ 创建基础UMG Blueprint

### 第2阶段：容器窗口（2-3小时）
1. ✅ 创建 `UGaiaContainerWindowWidget`
2. ✅ 集成容器网格
3. ✅ 实现容器数据绑定
4. ✅ 实现自动刷新
5. ✅ 创建容器窗口Blueprint

### 第3阶段：窗口管理（1-2小时）
1. ✅ 扩展 `UGaiaInventoryUIManager`
2. ✅ 实现多窗口管理
3. ✅ 实现窗口排列功能
4. ✅ 实现窗口布局保存/加载

### 第4阶段：增强功能（2-3小时）
1. ⏳ 实现窗口最小化
2. ⏳ 实现窗口吸附
3. ⏳ 实现窗口大小调整
4. ⏳ 实现窗口动画

### 第5阶段：测试和优化（1-2小时）
1. ⏳ 测试多窗口场景
2. ⏳ 测试网络同步
3. ⏳ 性能优化
4. ⏳ 视觉优化

## 🎯 使用示例

### C++使用
```cpp
// 获取UI管理器
UGaiaInventoryUIManager* UIManager = UGaiaInventoryUIManager::Get(GetWorld());

// 打开容器窗口
UGaiaContainerWindowWidget* Window = UIManager->OpenContainerWindow(ContainerUID);

// 设置窗口位置
Window->SetWindowPosition(FVector2D(100, 100));

// 关闭窗口
UIManager->CloseContainerWindow(Window);

// 排列所有窗口（级联）
UIManager->ArrangeWindowsCascade();
```

### Blueprint使用
```
# 打开容器窗口
Get Gaia Inventory UI Manager
    → Open Container Window
        Container UID: [容器UID]
        → Return: Container Window Widget

# 设置窗口位置
Container Window Widget
    → Set Window Position
        Position: (100, 100)

# 关闭窗口
Get Gaia Inventory UI Manager
    → Close Container Window
        Window: [窗口引用]
```

## 📊 性能考虑

### 1. 窗口数量限制
- 建议最大同时打开窗口数：10个
- 超过限制自动关闭最旧的窗口

### 2. 更新优化
- 只更新可见窗口
- 最小化窗口停止更新
- 使用事件驱动更新，避免Tick

### 3. 内存管理
- 关闭窗口时正确清理资源
- 使用对象池复用窗口Widget

## 🎨 视觉效果

### 动画
- **打开窗口**：从小到大+淡入（0.2秒）
- **关闭窗口**：从大到小+淡出（0.15秒）
- **最小化**：向标题栏折叠（0.2秒）
- **拖动**：平滑跟随鼠标

### 视觉反馈
- **悬停**：按钮高亮
- **点击**：按钮按下效果
- **拖动**：窗口半透明
- **聚焦**：边框高亮

## 🔄 后续扩展

### 可能的增强功能
1. 窗口停靠系统
2. 多标签窗口
3. 窗口分组
4. 窗口模板系统
5. 自定义窗口主题

---

**设计版本**: 1.0  
**最后更新**: 2025-10-31  
**状态**: 📋 设计完成，准备实现

