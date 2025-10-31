# GaiaDraggableTitleBar 使用指南

## 📋 概述

`GaiaDraggableTitleBar` 是一个通用的可拖动标题栏Widget基类，可以让任何使用它的Window Widget获得拖动能力。

### ✨ 核心特性

- ✅ **标题栏拖动**：鼠标拖动标题栏移动父Widget
- ✅ **关闭按钮**：内置关闭按钮和事件
- ✅ **双击事件**：可用于最大化/恢复
- ✅ **屏幕约束**：自动限制窗口在屏幕范围内
- ✅ **拖动视觉反馈**：拖动时自动降低透明度
- ✅ **完全可配置**：所有属性可在Blueprint中配置

## 🚀 快速开始

### 第1步：创建Blueprint

1. 在 Content Browser 中右键
2. **User Interface → Widget Blueprint**
3. 命名为 `WBP_DraggableTitleBar`
4. 父类选择 `GaiaDraggableTitleBar`

### 第2步：设计UI布局

在UMG Designer中创建以下组件结构：

```
Canvas Panel
  └─ [Border] TitleBarBorder (必须)
      └─ Horizontal Box
          ├─ [CommonTextBlock] TitleText (必须)
          ├─ Spacer
          └─ [CommonButtonBase] CloseButton (必须)
```

**重要：组件名称必须完全匹配！**

### 第3步：配置组件

#### TitleBarBorder
- **Type**: `Border`
- **Name**: `TitleBarBorder`
- **Is Variable**: ✅ 勾选
- **Brush Color**: `(0, 0, 0, 0.8)` 半透明黑色
- **Size**: 
  - Size To Content: Vertical
  - Height Override: 32

#### TitleText
- **Type**: `CommonTextBlock`
- **Name**: `TitleText`
- **Is Variable**: ✅ 勾选
- **Text**: `Window Title`
- **Font Size**: 14-16
- **Color**: 白色
- **Alignment**: Left + Center Vertically
- **Padding**: (8, 0, 0, 0)

#### CloseButton
- **Type**: `CommonButtonBase` 或其子类
- **Name**: `CloseButton`
- **Is Variable**: ✅ 勾选
- **Size**: 24x24 或 32x32
- **Text/Icon**: `×` 或关闭图标

### 第4步：使用标题栏

#### 在窗口Widget中使用

创建一个窗口Blueprint（例如 `WBP_MyWindow`）：

```
Canvas Panel
  └─ Vertical Box
      ├─ [WBP_DraggableTitleBar] TitleBar
      └─ Border (内容区域)
          └─ 你的内容...
```

#### C++中使用

```cpp
// 在你的窗口Widget类中
UCLASS()
class UMyWindowWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    // 标题栏
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UGaiaDraggableTitleBar* TitleBar;

protected:
    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();
        
        // 设置标题
        if (TitleBar)
        {
            TitleBar->SetTitleText(NSLOCTEXT("UI", "MyWindow", "我的窗口"));
            
            // 绑定关闭事件
            TitleBar->OnCloseClicked.AddDynamic(this, &UMyWindowWidget::HandleClose);
            
            // 绑定双击事件（可选）
            TitleBar->OnTitleBarDoubleClicked.AddDynamic(this, &UMyWindowWidget::HandleDoubleClick);
        }
    }

private:
    UFUNCTION()
    void HandleClose()
    {
        // 关闭窗口
        RemoveFromParent();
    }
    
    UFUNCTION()
    void HandleDoubleClick()
    {
        // 最大化/恢复
        ToggleMaximize();
    }
};
```

#### Blueprint中使用

1. **设置标题**：
   ```
   Event Construct
       ↓
   TitleBar → Set Title Text
       Text: "我的窗口"
   ```

2. **绑定关闭事件**：
   ```
   TitleBar → On Close Clicked
       ↓
   Remove From Parent (Self)
   ```

3. **绑定双击事件**：
   ```
   TitleBar → On Title Bar Double Clicked
       ↓
   Toggle Maximize (自定义)
   ```

## 🎨 样式自定义

### 在Blueprint中配置

选择 `TitleBar` 组件，在 Details 面板中：

#### Title Bar 分类
- **Default Title Text**: 默认标题文本
- **Is Draggable**: 是否可拖动（默认 true）
- **Show Close Button**: 是否显示关闭按钮（默认 true）
- **Drag Opacity**: 拖动时透明度（0.0-1.0，默认 0.7）
- **Constrain To Screen**: 是否限制在屏幕内（默认 true）

### 视觉样式示例

#### 现代深色主题
```
TitleBarBorder:
  - Brush Color: (20, 20, 25, 0.95)
  - Border: 金色 (255, 215, 0, 1.0), 1px

TitleText:
  - Color: (240, 240, 240)
  - Font Size: 14
  - Font: Bold

CloseButton:
  - Normal: 半透明红色
  - Hovered: 亮红色
  - Size: 28x28
```

#### 游戏风格主题
```
TitleBarBorder:
  - Brush: 使用纹理
  - Tiling: Horizontal

TitleText:
  - Color: 金色
  - Font: 游戏字体
  - Outline: 深色描边

CloseButton:
  - Style: 图标按钮
  - Icon: 剑形X
```

## 📚 API参考

### 公开函数

#### SetTitleText
```cpp
UFUNCTION(BlueprintCallable, Category = "Title Bar")
void SetTitleText(const FText& InTitleText);
```
设置标题文本。

**参数**：
- `InTitleText` - 新的标题文本

**示例**：
```cpp
TitleBar->SetTitleText(NSLOCTEXT("UI", "Inventory", "库存"));
```

#### GetTitleText
```cpp
UFUNCTION(BlueprintPure, Category = "Title Bar")
FText GetTitleText() const;
```
获取当前标题文本。

#### SetCloseButtonVisible
```cpp
UFUNCTION(BlueprintCallable, Category = "Title Bar")
void SetCloseButtonVisible(bool bVisible);
```
设置关闭按钮可见性。

**参数**：
- `bVisible` - 是否显示

#### SetDraggable
```cpp
UFUNCTION(BlueprintCallable, Category = "Title Bar")
void SetDraggable(bool bCanDrag);
```
设置是否可拖动。

**参数**：
- `bCanDrag` - 是否可拖动

#### IsDragging
```cpp
UFUNCTION(BlueprintPure, Category = "Title Bar")
bool IsDragging() const;
```
检查是否正在拖动。

**返回值**：
- `true` - 正在拖动
- `false` - 未拖动

### 事件委托

#### OnCloseClicked
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseClicked);
UPROPERTY(BlueprintAssignable, Category = "Title Bar|Events")
FOnCloseClicked OnCloseClicked;
```
关闭按钮被点击时触发。

**示例**：
```cpp
TitleBar->OnCloseClicked.AddDynamic(this, &UMyWidget::OnClose);
```

#### OnDragStarted
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDragStarted);
UPROPERTY(BlueprintAssignable, Category = "Title Bar|Events")
FOnDragStarted OnDragStarted;
```
开始拖动时触发。

#### OnDragEnded
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDragEnded);
UPROPERTY(BlueprintAssignable, Category = "Title Bar|Events")
FOnDragEnded OnDragEnded;
```
拖动结束时触发。

#### OnTitleBarDoubleClicked
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTitleBarDoubleClicked);
UPROPERTY(BlueprintAssignable, Category = "Title Bar|Events")
FOnTitleBarDoubleClicked OnTitleBarDoubleClicked;
```
双击标题栏时触发（通常用于最大化/恢复）。

## 💡 使用示例

### 示例1：简单窗口

```cpp
UCLASS()
class USimpleWindow : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UGaiaDraggableTitleBar* TitleBar;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* ContentText;

protected:
    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();
        
        if (TitleBar)
        {
            TitleBar->SetTitleText(NSLOCTEXT("UI", "Hello", "Hello Window"));
            TitleBar->OnCloseClicked.AddDynamic(this, &USimpleWindow::CloseWindow);
        }
    }

private:
    UFUNCTION()
    void CloseWindow()
    {
        RemoveFromParent();
    }
};
```

### 示例2：容器窗口

```cpp
UCLASS()
class UContainerWindow : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UGaiaDraggableTitleBar* TitleBar;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UGaiaContainerGridWidget* ContainerGrid;

    void InitializeForContainer(const FGuid& ContainerUID)
    {
        CurrentContainerUID = ContainerUID;
        
        // 更新标题
        if (TitleBar)
        {
            FString ContainerName = GetContainerName(ContainerUID);
            TitleBar->SetTitleText(FText::FromString(ContainerName));
        }
        
        // 初始化网格
        if (ContainerGrid)
        {
            ContainerGrid->InitializeForContainer(ContainerUID);
        }
    }

protected:
    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();
        
        if (TitleBar)
        {
            TitleBar->OnCloseClicked.AddDynamic(this, &UContainerWindow::CloseWindow);
            TitleBar->OnDragStarted.AddDynamic(this, &UContainerWindow::OnWindowDragStarted);
            TitleBar->OnDragEnded.AddDynamic(this, &UContainerWindow::OnWindowDragEnded);
        }
    }

private:
    FGuid CurrentContainerUID;
    
    UFUNCTION()
    void CloseWindow()
    {
        DeactivateWidget();
    }
    
    UFUNCTION()
    void OnWindowDragStarted()
    {
        // 拖动开始时的逻辑
        UE_LOG(LogTemp, Log, TEXT("Window drag started"));
    }
    
    UFUNCTION()
    void OnWindowDragEnded()
    {
        // 拖动结束时的逻辑（例如保存位置）
        SaveWindowPosition();
    }
    
    void SaveWindowPosition()
    {
        // 保存窗口位置到配置文件
    }
};
```

### 示例3：最大化/最小化窗口

```cpp
UCLASS()
class UAdvancedWindow : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UGaiaDraggableTitleBar* TitleBar;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UBorder* ContentBorder;

protected:
    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();
        
        if (TitleBar)
        {
            TitleBar->OnTitleBarDoubleClicked.AddDynamic(this, &UAdvancedWindow::ToggleMaximize);
        }
    }

private:
    bool bIsMaximized = false;
    FVector2D SavedSize;
    FVector2D SavedPosition;
    
    UFUNCTION()
    void ToggleMaximize()
    {
        if (bIsMaximized)
        {
            Restore();
        }
        else
        {
            Maximize();
        }
    }
    
    void Maximize()
    {
        // 保存当前大小和位置
        SavedSize = GetDesiredSize();
        SavedPosition = GetCachedGeometry().GetAbsolutePosition();
        
        // 获取视口大小
        FVector2D ViewportSize;
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        
        // 最大化
        SetPositionInViewport(FVector2D::ZeroVector, false);
        SetDesiredSizeInViewport(ViewportSize);
        
        // 禁用拖动
        if (TitleBar)
        {
            TitleBar->SetDraggable(false);
        }
        
        bIsMaximized = true;
    }
    
    void Restore()
    {
        // 恢复原始大小和位置
        SetPositionInViewport(SavedPosition, false);
        SetDesiredSizeInViewport(SavedSize);
        
        // 启用拖动
        if (TitleBar)
        {
            TitleBar->SetDraggable(true);
        }
        
        bIsMaximized = false;
    }
};
```

## 🎯 最佳实践

### 1. 命名约定
- 统一使用 `TitleBar` 作为标题栏变量名
- 使用描述性的标题文本
- 使用本地化文本（NSLOCTEXT）

### 2. 事件处理
- 始终绑定 `OnCloseClicked` 事件
- 考虑使用 `OnDragEnded` 保存窗口位置
- 使用 `OnTitleBarDoubleClicked` 实现最大化

### 3. 性能优化
- 避免在拖动过程中执行昂贵的操作
- 使用事件而非Tick
- 合理使用屏幕约束

### 4. 用户体验
- 提供清晰的视觉反馈
- 确保窗口始终可见
- 支持快捷键操作

## ⚠️ 常见问题

### Q1: 窗口无法拖动
**A**: 检查：
- `bIsDraggable` 是否为 `true`
- 组件名称是否正确（`TitleBarBorder`, `TitleText`, `CloseButton`）
- 标题栏是否被其他组件覆盖

### Q2: 拖动时窗口闪烁
**A**: 
- 降低 `DragOpacity` 值
- 检查父Widget的刷新频率
- 使用 `SetPositionInViewport` 而非 `SetRenderTranslation`

### Q3: 窗口拖出屏幕
**A**: 
- 确保 `bConstrainToScreen` 为 `true`
- 检查视口大小获取是否正确

### Q4: 双击事件不触发
**A**: 
- 确保绑定了 `OnTitleBarDoubleClicked`
- 检查是否有其他组件拦截了点击事件

## 📈 下一步

### 扩展功能
1. ✅ 添加最小化按钮
2. ✅ 添加最大化按钮
3. ✅ 支持窗口吸附
4. ✅ 支持多显示器
5. ✅ 添加窗口动画

### 集成示例
- 集成到容器窗口
- 集成到对话框
- 集成到设置面板

---

**版本**: 1.0  
**最后更新**: 2025-10-31  
**文件位置**: `Source/Gaia/UI/GaiaDraggableTitleBar.h/.cpp`  
**状态**: ✅ 已完成并编译通过

