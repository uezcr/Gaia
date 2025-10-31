# GaiaButton 使用指南

## 📖 简介

`UGaiaButton` 是 Gaia 项目的通用按钮基类，继承自 CommonUI 的 `UCommonButtonBase`，提供开箱即用的样式、动画和功能。

### 核心特性

- ✅ **8种预设样式**：Primary, Secondary, Success, Danger, Warning, Info, Transparent, Custom
- ✅ **4种尺寸预设**：Small (24px), Medium (32px), Large (40px), XLarge (48px)
- ✅ **状态管理**：加载状态、徽章显示、禁用状态
- ✅ **动画效果**：脉冲动画、点击反馈
- ✅ **声音集成**：悬停和点击声音
- ✅ **蓝图友好**：所有功能可在蓝图中使用
- ✅ **CommonUI集成**：支持键盘导航和手柄输入

---

## 🚀 快速开始

### 第一步：创建 Widget 蓝图

1. 在内容浏览器中，右键 → **用户界面** → **Widget 蓝图**
2. 选择父类：**GaiaButton**
3. 命名为 `WBP_MyButton`

### 第二步：构建 UI 层级

在 Designer 面板中创建以下层级结构：

```
[Overlay] Root
├─ [Border] Background (必须)
├─ [Overlay] Content (必须)
│   ├─ [Image] Icon (可选)
│   ├─ [TextBlock] ButtonText (可选)
│   └─ [ProgressBar] LoadingIndicator (可选)
└─ [Border] Badge (可选)
    └─ [TextBlock] BadgeText (可选)
```

**重要**：
- 组件名称必须与上述一致（通过 `BindWidget` 自动绑定）
- `Background` 和 `Content` 是必须的
- 其他组件是可选的，根据需要添加

### 第三步：配置按钮

在 Details 面板中设置：

| 属性 | 说明 | 默认值 |
|------|------|--------|
| **Current Style** | 按钮样式 | Primary |
| **Current Size** | 按钮尺寸 | Medium |
| **Hover Sound** | 悬停声音 | None |
| **Click Sound** | 点击声音 | None |

### 第四步：使用按钮

**在蓝图中**：

```
Event Construct
  ↓
Get Widget → WBP_MyButton
  ↓
Set Button Text → "确认"
  ↓
Set Button Style → Primary
  ↓
Bind Event to On Clicked
  ↓
Print String → "按钮被点击！"
```

**在 C++ 中**：

```cpp
UGaiaButton* MyButton = CreateWidget<UGaiaButton>(this, WBP_MyButtonClass);
MyButton->SetButtonText(FText::FromString(TEXT("确认")));
MyButton->SetButtonStyle(EGaiaButtonStyle::Primary);
MyButton->SetButtonSize(EGaiaButtonSize::Medium);
MyButton->OnClicked().AddUObject(this, &UMyClass::OnButtonClicked);
MyParentWidget->AddChild(MyButton);
```

---

## 🎨 样式系统

### 预设样式

#### Primary（主要按钮）
- **颜色**：蓝色 (0.2, 0.5, 1.0)
- **用途**：主要操作（确认、提交、开始游戏等）
- **示例**：
```cpp
MyButton->SetButtonStyle(EGaiaButtonStyle::Primary);
```

#### Secondary（次要按钮）
- **颜色**：灰色 (0.3, 0.3, 0.3)
- **用途**：次要操作（取消、返回等）
- **示例**：
```cpp
MyButton->SetButtonStyle(EGaiaButtonStyle::Secondary);
```

#### Success（成功按钮）
- **颜色**：绿色 (0.2, 0.8, 0.2)
- **用途**：成功确认操作（保存成功、完成等）
- **示例**：
```cpp
MyButton->SetButtonStyle(EGaiaButtonStyle::Success);
```

#### Danger（危险按钮）
- **颜色**：红色 (0.9, 0.2, 0.2)
- **用途**：危险操作（删除、退出、重置等）
- **示例**：
```cpp
MyButton->SetButtonStyle(EGaiaButtonStyle::Danger);
```

#### Warning（警告按钮）
- **颜色**：黄色 (1.0, 0.7, 0.0)
- **用途**：警告操作（需要注意的操作）
- **示例**：
```cpp
MyButton->SetButtonStyle(EGaiaButtonStyle::Warning);
```

#### Info（信息按钮）
- **颜色**：青色 (0.2, 0.8, 0.8)
- **用途**：信息展示操作（帮助、详情等）
- **示例**：
```cpp
MyButton->SetButtonStyle(EGaiaButtonStyle::Info);
```

#### Transparent（透明按钮）
- **颜色**：透明 (0.0, 0.0, 0.0, 0.0)
- **用途**：悬浮按钮、图标按钮
- **示例**：
```cpp
MyButton->SetButtonStyle(EGaiaButtonStyle::Transparent);
```

#### Custom（自定义样式）
- **颜色**：自定义
- **用途**：特殊需求
- **示例**：
```cpp
MyButton->SetButtonStyle(EGaiaButtonStyle::Custom);
MyButton->SetCustomColors(
    FLinearColor(1.0f, 0.0f, 1.0f, 1.0f),  // Normal: 紫色
    FLinearColor(1.0f, 0.5f, 1.0f, 1.0f),  // Hovered
    FLinearColor(0.8f, 0.0f, 0.8f, 1.0f),  // Pressed
    FLinearColor(0.3f, 0.3f, 0.3f, 0.5f)   // Disabled
);
```

### 自动状态颜色

按钮会根据状态自动应用不同的颜色：

| 状态 | 颜色变化 |
|------|----------|
| **Normal** | 基础颜色 |
| **Hovered** | 稍微变亮 |
| **Pressed** | 稍微变暗 |
| **Disabled** | 灰色半透明 |

---

## 📏 尺寸系统

### 预设尺寸

| 尺寸 | 高度 | 最小宽度 | 字体大小 | 图标大小 | 内边距 |
|------|------|----------|----------|----------|--------|
| **Small** | 24px | 64px | 12 | 12px | (12, 4) |
| **Medium** | 32px | 80px | 14 | 16px | (16, 8) |
| **Large** | 40px | 96px | 16 | 20px | (20, 10) |
| **XLarge** | 48px | 112px | 18 | 24px | (24, 12) |

### 使用示例

```cpp
// 小按钮
MyButton->SetButtonSize(EGaiaButtonSize::Small);

// 中等按钮（默认）
MyButton->SetButtonSize(EGaiaButtonSize::Medium);

// 大按钮
MyButton->SetButtonSize(EGaiaButtonSize::Large);

// 超大按钮
MyButton->SetButtonSize(EGaiaButtonSize::XLarge);
```

---

## 🎭 状态管理

### 加载状态

显示加载动画，自动禁用按钮并隐藏文本/图标：

```cpp
// 开始加载
MyButton->SetLoading(true);

// 模拟异步操作
AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
{
    // 执行耗时操作...
    FPlatformProcess::Sleep(2.0f);
    
    // 完成后在主线程恢复
    AsyncTask(ENamedThreads::GameThread, [this]()
    {
        MyButton->SetLoading(false);
    });
});
```

### 徽章显示

显示通知数量（如未读消息数）：

```cpp
// 显示徽章
MyButton->SetBadgeCount(5);  // 显示 "5"

// 大数量显示为 "99+"
MyButton->SetBadgeCount(150);  // 显示 "99+"

// 隐藏徽章
MyButton->SetBadgeCount(0);
```

### 工具提示

```cpp
MyButton->SetTooltipText(FText::FromString(TEXT("点击此按钮执行操作")));
```

### 禁用/启用

```cpp
// 禁用按钮
MyButton->SetIsEnabled(false);

// 启用按钮
MyButton->SetIsEnabled(true);
```

---

## 🎬 动画系统

### 脉冲动画

吸引用户注意力的脉冲动画：

```cpp
// 开始脉冲动画
MyButton->PlayPulseAnimation();

// 停止脉冲动画
MyButton->StopPulseAnimation();
```

**配置参数**：

```cpp
// 在蓝图的 Details 面板中设置：
- Enable Pulse Animation: true
- Pulse Speed: 1.0 (动画速度)
- Pulse Scale: 1.1 (缩放范围，1.0 - 2.0)
```

### 点击反馈动画

点击时自动播放（无需手动调用）：

```cpp
// 按钮被点击时自动调用
MyButton->PlayClickAnimation();
```

---

## 🔊 声音效果

### 设置声音

**在蓝图中**：

1. 选择按钮
2. Details → Sound
3. 设置 `Hover Sound` 和 `Click Sound`

**在 C++ 中**：

```cpp
// 加载声音资源
USoundBase* HoverSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/UI_Hover.UI_Hover"));
USoundBase* ClickSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/UI_Click.UI_Click"));

// 设置到按钮
MyButton->HoverSound = HoverSound;
MyButton->ClickSound = ClickSound;
```

### 声音触发时机

- **HoverSound**：鼠标进入按钮时播放
- **ClickSound**：按钮被点击时播放

---

## 🎯 高级用法

### 组合使用

```cpp
// 创建一个完整配置的按钮
UGaiaButton* SaveButton = CreateWidget<UGaiaButton>(this, ButtonClass);

// 样式和内容
SaveButton->SetButtonStyle(EGaiaButtonStyle::Success);
SaveButton->SetButtonSize(EGaiaButtonSize::Large);
SaveButton->SetButtonText(FText::FromString(TEXT("保存进度")));

// 图标
UTexture2D* SaveIcon = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Icons/Save.Save"));
SaveButton->SetButtonIcon(SaveIcon);

// 徽章（显示未保存的更改数量）
SaveButton->SetBadgeCount(3);

// 工具提示
SaveButton->SetTooltipText(FText::FromString(TEXT("保存当前游戏进度（Ctrl+S）")));

// 声音
SaveButton->HoverSound = HoverSound;
SaveButton->ClickSound = ClickSound;

// 脉冲动画（吸引注意）
SaveButton->PlayPulseAnimation();

// 事件绑定
SaveButton->OnClicked().AddUObject(this, &UMyClass::OnSaveClicked);

// 添加到界面
MyPanel->AddChild(SaveButton);
```

### 异步操作示例

```cpp
void UMyWidget::OnSaveClicked()
{
    // 开始加载状态
    SaveButton->SetLoading(true);
    
    // 异步保存数据
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
    {
        // 模拟保存操作
        SaveGameData();
        
        // 完成后返回主线程
        AsyncTask(ENamedThreads::GameThread, [this]()
        {
            SaveButton->SetLoading(false);
            SaveButton->SetButtonStyle(EGaiaButtonStyle::Success);
            SaveButton->SetButtonText(FText::FromString(TEXT("保存成功！")));
            
            // 2秒后恢复原始文本
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
            {
                SaveButton->SetButtonText(FText::FromString(TEXT("保存进度")));
            }, 2.0f, false);
        });
    });
}
```

### 动态切换样式

```cpp
// 根据条件动态切换样式
void UMyWidget::UpdateButtonState(bool bIsValid)
{
    if (bIsValid)
    {
        MyButton->SetButtonStyle(EGaiaButtonStyle::Success);
        MyButton->SetButtonText(FText::FromString(TEXT("有效")));
        MyButton->SetIsEnabled(true);
    }
    else
    {
        MyButton->SetButtonStyle(EGaiaButtonStyle::Danger);
        MyButton->SetButtonText(FText::FromString(TEXT("无效")));
        MyButton->SetIsEnabled(false);
    }
}
```

---

## 📋 完整 API 参考

### 样式相关

```cpp
// 设置样式
void SetButtonStyle(EGaiaButtonStyle Style);
EGaiaButtonStyle GetButtonStyle() const;

// 设置尺寸
void SetButtonSize(EGaiaButtonSize Size);
EGaiaButtonSize GetButtonSize() const;

// 自定义颜色（仅 Custom 样式）
void SetCustomColors(FLinearColor Normal, FLinearColor Hovered, 
                     FLinearColor Pressed, FLinearColor Disabled);
```

### 内容相关

```cpp
// 文本
void SetButtonText(const FText& Text);
FText GetButtonText() const;

// 图标
void SetButtonIcon(UTexture2D* IconTexture);
void SetIconVisible(bool bVisible);
```

### 状态相关

```cpp
// 加载状态
void SetLoading(bool bIsLoading);
bool IsLoading() const;

// 徽章
void SetBadgeCount(int32 Count);
int32 GetBadgeCount() const;

// 工具提示
void SetTooltipText(const FText& TooltipText);

// 启用/禁用（继承自 UCommonButtonBase）
void SetIsEnabled(bool bInIsEnabled);
bool GetIsEnabled() const;
```

### 动画相关

```cpp
// 脉冲动画
void PlayPulseAnimation();
void StopPulseAnimation();

// 点击动画
void PlayClickAnimation();
```

### 事件

```cpp
// 继承自 UCommonButtonBase 的事件
FCommonButtonBaseClicked OnClicked;
FCommonButtonBaseClicked OnPressed;
FCommonButtonBaseClicked OnReleased;
FCommonButtonBaseClicked OnHovered;
FCommonButtonBaseClicked OnUnhovered;
```

---

## 💡 最佳实践

### 1. **样式选择原则**

- ✅ **Primary**：用于页面的主要操作（1个）
- ✅ **Secondary**：用于次要操作（多个）
- ✅ **Danger**：用于不可逆操作（删除、退出）
- ✅ **Success/Warning/Info**：用于状态反馈

### 2. **尺寸使用建议**

- ✅ **Small**：工具栏、列表项
- ✅ **Medium**：对话框、表单（默认选择）
- ✅ **Large**：主菜单、重要操作
- ✅ **XLarge**：启动界面、特大操作

### 3. **性能优化**

- ✅ 复用按钮实例，避免频繁创建销毁
- ✅ 只在需要时启用脉冲动画
- ✅ 声音资源使用引用而非每次加载

### 4. **用户体验**

- ✅ 为长时间操作显示加载状态
- ✅ 使用徽章显示重要通知
- ✅ 提供工具提示说明按钮功能
- ✅ 为危险操作二次确认

---

## 🐛 常见问题

### Q: 按钮没有显示样式？

**A**: 检查 UMG 层级结构是否正确：
- 确保有 `Background` 和 `Content` 组件
- 组件名称必须与代码中的 `BindWidget` 一致
- 在 Designer 中编译蓝图

### Q: 点击按钮没有响应？

**A**: 检查以下几点：
- 按钮是否启用（`SetIsEnabled(true)`）
- 是否正确绑定了 `OnClicked` 事件
- 按钮是否在视口中可见
- 父控件是否拦截了输入事件

### Q: 加载动画不显示？

**A**: 确保在 UMG 中添加了 `LoadingIndicator` 组件（ProgressBar）

### Q: 徽章不显示？

**A**: 确保在 UMG 中添加了 `Badge` 和 `BadgeText` 组件

### Q: 自定义样式不生效？

**A**: 确保将 `CurrentStyle` 设置为 `Custom`，然后调用 `SetCustomColors()`

---

## 🎓 示例项目

查看以下位置的示例：

- **蓝图示例**：`Content/UI/Examples/WBP_ButtonExamples`
- **C++ 示例**：`Source/Gaia/UI/Examples/GaiaButtonExamples.h`
- **测试关卡**：`Content/Maps/UI_TestMap`

---

## 📚 相关文档

- [CommonButton分析与设计](./CommonButton分析与设计.md)
- [CommonUI 官方文档](https://docs.unrealengine.com/5.3/en-US/common-ui-plugin-for-advanced-input-in-unreal-engine/)
- [UMG 用户界面指南](https://docs.unrealengine.com/5.3/en-US/umg-ui-designer-for-unreal-engine/)

