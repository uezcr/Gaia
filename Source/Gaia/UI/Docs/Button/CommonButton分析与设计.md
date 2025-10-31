# CommonButton 分析与 GaiaButton 设计

## 📋 CommonButtonBase 核心特性分析

### 1. **状态管理系统**

CommonButtonBase 提供了完整的按钮状态管理：

```cpp
enum class ECommonButtonState : uint8
{
    Normal,      // 正常状态
    Hovered,     // 鼠标悬停
    Pressed,     // 按下
    Disabled,    // 禁用
    Selected     // 选中（用于单选/多选）
};
```

**关键功能**：
- ✅ 自动状态切换（Normal → Hovered → Pressed）
- ✅ 禁用状态管理
- ✅ 选中状态支持（Toggle按钮）
- ✅ 状态变化时触发事件

### 2. **样式系统**

CommonButtonBase 使用 `UCommonButtonStyle` 数据资产管理样式：

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Style")
TSubclassOf<UCommonButtonStyle> Style;
```

**样式包含**：
- 每个状态的背景图片/颜色
- 文本样式（字体、大小、颜色）
- 内边距（Padding）
- 声音效果（点击、悬停）

### 3. **输入处理**

```cpp
// 虚函数 - 子类可重写
virtual void HandleButtonClicked();
virtual void HandleButtonPressed();
virtual void HandleButtonReleased();
virtual void HandleButtonHovered();
virtual void HandleButtonUnhovered();
```

**特点**：
- ✅ 支持键盘导航（CommonUI特性）
- ✅ 支持手柄输入
- ✅ 输入焦点管理
- ✅ 自动处理输入路由

### 4. **事件系统**

```cpp
UPROPERTY(BlueprintAssignable, Category = "Button")
FCommonButtonBaseClicked OnClicked;

UPROPERTY(BlueprintAssignable, Category = "Button")
FCommonButtonBaseClicked OnPressed;

UPROPERTY(BlueprintAssignable, Category = "Button")
FCommonButtonBaseClicked OnReleased;

UPROPERTY(BlueprintAssignable, Category = "Button")
FCommonButtonBaseClicked OnHovered;

UPROPERTY(BlueprintAssignable, Category = "Button")
FCommonButtonBaseClicked OnUnhovered;

UPROPERTY(BlueprintAssignable, Category = "Button")
FCommonSelectedStateChanged OnSelectedChanged;
```

### 5. **CommonUI 集成**

- ✅ 继承自 `UCommonUserWidget`
- ✅ 自动参与 CommonUI 输入路由
- ✅ 支持 CommonActivatableWidget 激活系统
- ✅ 与 CommonInputActionDataBase 集成

---

## 🎨 GaiaButton 设计方案

### 设计目标

1. **简化使用**：提供开箱即用的常见按钮类型
2. **样式一致**：统一的 Gaia UI 风格
3. **扩展性强**：易于创建自定义按钮
4. **性能优化**：避免不必要的状态更新
5. **蓝图友好**：所有功能可在蓝图中访问

### 类层次结构

```
UCommonButtonBase (引擎)
    ↓
UGaiaButton (通用基类) ← 新创建
    ↓
    ├─ UGaiaIconButton (图标按钮)
    ├─ UGaiaTextButton (文本按钮)
    ├─ UGaiaImageButton (图片按钮)
    └─ UGaiaContextMenuButton (已存在，菜单按钮)
```

---

## 📝 UGaiaButton 功能设计

### 核心功能

#### 1. **快速样式配置**

```cpp
// 预设样式枚举
UENUM(BlueprintType)
enum class EGaiaButtonStyle : uint8
{
    Primary,      // 主要按钮（高亮，蓝色）
    Secondary,    // 次要按钮（灰色）
    Success,      // 成功按钮（绿色）
    Danger,       // 危险按钮（红色）
    Warning,      // 警告按钮（黄色）
    Info,         // 信息按钮（青色）
    Transparent,  // 透明按钮
    Custom        // 自定义样式
};

// 快速设置样式
UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
void SetButtonStyle(EGaiaButtonStyle Style);
```

#### 2. **尺寸管理**

```cpp
UENUM(BlueprintType)
enum class EGaiaButtonSize : uint8
{
    Small,    // 24px 高度
    Medium,   // 32px 高度
    Large,    // 40px 高度
    XLarge    // 48px 高度
};

UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
void SetButtonSize(EGaiaButtonSize Size);
```

#### 3. **状态管理增强**

```cpp
// 加载状态（用于异步操作）
UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
void SetLoading(bool bIsLoading);

// 徽章/通知数量
UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
void SetBadgeCount(int32 Count);

// 工具提示
UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
void SetTooltipText(const FText& TooltipText);
```

#### 4. **动画支持**

```cpp
// 脉冲动画（吸引注意力）
UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
void PlayPulseAnimation();

// 点击反馈动画
UFUNCTION(BlueprintCallable, Category = "Gaia|Button")
void PlayClickAnimation();
```

#### 5. **声音效果**

```cpp
// 悬停声音
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Sound")
USoundBase* HoverSound;

// 点击声音
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaia|Sound")
USoundBase* ClickSound;
```

### UMG 结构

```
[Overlay] Root
├─ [Border] Background
│   └─ [Image] BackgroundImage
├─ [Border] Content
│   ├─ [Image] Icon (可选)
│   ├─ [TextBlock] Text (可选)
│   └─ [ProgressBar] LoadingIndicator (可选)
└─ [Border] Badge
    └─ [TextBlock] BadgeText
```

---

## 🔧 实现要点

### 1. **样式数据表**

创建 DataTable 存储预设样式：

```cpp
USTRUCT(BlueprintType)
struct FGaiaButtonStyleData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor NormalColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HoveredColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor PressedColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DisabledColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSlateFontInfo FontInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMargin Padding;
};
```

### 2. **性能优化**

- 使用 `TObjectPtr<>` 代替原始指针
- 状态变化时才重绘（避免每帧更新）
- 复用 Slate 画刷（Brush）对象
- 延迟加载纹理资源

### 3. **可访问性**

- 支持键盘 Tab 导航
- 支持屏幕阅读器（Screen Reader）
- 提供高对比度模式
- 支持自定义热键绑定

---

## 📊 与现有按钮的对比

| 特性 | UButton (UMG) | UCommonButtonBase | UGaiaButton |
|------|---------------|-------------------|-------------|
| 状态管理 | ✅ 基础 | ✅ 完整 | ✅ 增强 |
| 样式系统 | ❌ 手动配置 | ✅ 样式资产 | ✅ 预设+自定义 |
| 输入路由 | ❌ | ✅ CommonUI | ✅ CommonUI |
| 动画支持 | ❌ | ❌ | ✅ 内置动画 |
| 加载状态 | ❌ | ❌ | ✅ |
| 徽章显示 | ❌ | ❌ | ✅ |
| 声音效果 | ❌ | ✅ | ✅ 增强 |
| 快速样式 | ❌ | ❌ | ✅ 枚举选择 |

---

## 🎯 使用示例

### C++ 使用

```cpp
// 创建主要按钮
UGaiaButton* ConfirmButton = CreateWidget<UGaiaButton>(this);
ConfirmButton->SetButtonStyle(EGaiaButtonStyle::Primary);
ConfirmButton->SetButtonSize(EGaiaButtonSize::Medium);
ConfirmButton->SetButtonText(FText::FromString("确认"));
ConfirmButton->OnClicked.AddDynamic(this, &UMyWidget::OnConfirmClicked);

// 创建带加载状态的按钮
UGaiaButton* SaveButton = CreateWidget<UGaiaButton>(this);
SaveButton->SetButtonText(FText::FromString("保存"));
SaveButton->SetLoading(true);  // 显示加载动画
```

### 蓝图使用

```
[Begin Play]
  ↓
[Create Widget] → GaiaButton
  ↓
[Set Button Style] → Primary
  ↓
[Set Button Size] → Medium
  ↓
[Set Button Text] → "开始游戏"
  ↓
[Add to Viewport]
```

---

## 🚀 实现计划

### 第一阶段：基础功能
- ✅ 创建 `UGaiaButton` 基类
- ✅ 实现样式系统（枚举 + 配置）
- ✅ 实现尺寸系统
- ✅ 集成声音效果

### 第二阶段：增强功能
- ⏳ 加载状态动画
- ⏳ 徽章显示
- ⏳ 工具提示
- ⏳ 快捷键绑定

### 第三阶段：扩展类
- ⏳ `UGaiaIconButton`
- ⏳ `UGaiaTextButton`
- ⏳ `UGaiaImageButton`
- ⏳ 重构现有 `UGaiaContextMenuButton`

### 第四阶段：样式资产
- ⏳ 创建默认样式 DataTable
- ⏳ 创建蓝图样式预设
- ⏳ 创建编辑器工具（快速生成样式）

---

## 📚 参考资源

- [CommonUI Documentation](https://docs.unrealengine.com/5.3/en-US/common-ui-plugin-for-advanced-input-in-unreal-engine/)
- [UCommonButtonBase Source](Engine/Plugins/Experimental/CommonUI/Source/CommonUI/Public/CommonButtonBase.h)
- [Material Design Button Guidelines](https://m3.material.io/components/buttons/overview)
- [Gaia 项目现有按钮](Source/Gaia/UI/Inventory/GaiaContextMenuButton.h)

