# Gaia UI 系统

Gaia 项目的 UI 系统，基于 Unreal Engine 的 CommonUI 框架构建。

## 📖 文档

所有详细文档都位于 [`Docs/`](./Docs/) 目录中。

### 快速链接

#### 按钮系统
- [GaiaButton 使用指南](./Docs/Button/GaiaButton使用指南.md) - 完整的使用指南
- [CommonButton 分析与设计](./Docs/Button/CommonButton分析与设计.md) - 设计方案

#### 窗口系统
- [Canvas 窗口系统使用指南](./Docs/Window/Canvas窗口系统使用指南.md) - 架构说明
- [GaiaDraggableWindow 使用指南](./Docs/Window/GaiaDraggableWindow使用指南.md) - 窗口使用
- [GaiaDraggableTitleBar 使用指南](./Docs/Window/GaiaDraggableTitleBar使用指南.md) - 标题栏使用

### 完整文档索引
查看 [Docs/README.md](./Docs/README.md) 获取所有文档的完整列表。

---

## 🚀 快速开始

### 创建一个按钮

```cpp
// 在 C++ 中
UGaiaButton* MyButton = CreateWidget<UGaiaButton>(this, ButtonClass);
MyButton->SetButtonStyle(EGaiaButtonStyle::Primary);
MyButton->SetButtonSize(EGaiaButtonSize::Medium);
MyButton->SetButtonText(FText::FromString(TEXT("确认")));
MyButton->OnClicked().AddUObject(this, &UMyClass::OnButtonClicked);
```

### 创建一个窗口

```cpp
// 在 C++ 中
UGaiaDraggableWindow* MyWindow = CreateWidget<UGaiaDraggableWindow>(this, WindowClass);
MyWindow->SetWindowTitle(FText::FromString(TEXT("背包")));
MyWindow->ShowInCanvas(MainCanvas, FVector2D(100, 100));
```

---

## 📦 核心组件

### 通用组件

| 组件 | 文件 | 说明 |
|------|------|------|
| **GaiaButton** | `GaiaButton.h/cpp` | 通用按钮，8种预设样式 |
| **GaiaDraggableWindow** | `GaiaDraggableWindow.h/cpp` | 可拖拽窗口 |
| **GaiaDraggableTitleBar** | `GaiaDraggableTitleBar.h/cpp` | 可拖拽标题栏 |

### 系统组件

| 组件 | 文件 | 说明 |
|------|------|------|
| **GaiaGameHUD** | `GaiaGameHUD.h/cpp` | 游戏 HUD 管理 |
| **GaiaUIManagerSubsystem** | `GaiaUIManagerSubsystem.h/cpp` | UI 管理子系统 |
| **GaiaPrimaryGameLayout** | `GaiaPrimaryGameLayout.h/cpp` | 主游戏布局 |

### 库存 UI

| 组件 | 文件 | 说明 |
|------|------|------|
| **GaiaItemSlotWidget** | `Inventory/GaiaItemSlotWidget.h/cpp` | 物品槽位 |
| **GaiaContainerGridWidget** | `Inventory/GaiaContainerGridWidget.h/cpp` | 容器网格 |
| **GaiaContainerWindowWidget** | `Inventory/GaiaContainerWindowWidget.h/cpp` | 容器窗口 |
| **GaiaItemContextMenu** | `Inventory/GaiaItemContextMenu.h/cpp` | 右键菜单 |

---

## 🎨 设计特点

### CommonUI 集成
- ✅ 支持键盘/手柄导航
- ✅ 自动输入路由管理
- ✅ 焦点系统

### 模块化架构
- ✅ 组件职责单一
- ✅ 易于组合扩展
- ✅ 低耦合高内聚

### 蓝图友好
- ✅ 所有功能可在蓝图使用
- ✅ 详细的工具提示
- ✅ 预设枚举选择

### 性能优化
- ✅ 智能指针管理
- ✅ 状态驱动更新
- ✅ 避免不必要拷贝

---

## 📁 目录结构

```
UI/
├── Docs/                          # 所有文档
│   ├── Button/                    # 按钮系统文档
│   ├── Window/                    # 窗口系统文档
│   ├── Archive/                   # 归档文档
│   └── README.md                  # 文档索引
├── Inventory/                     # 库存 UI 组件
├── GaiaButton.h/cpp               # 通用按钮
├── GaiaDraggableWindow.h/cpp      # 可拖拽窗口
├── GaiaDraggableTitleBar.h/cpp    # 可拖拽标题栏
├── GaiaGameHUD.h/cpp              # 游戏 HUD
├── GaiaUIManagerSubsystem.h/cpp   # UI 管理子系统
└── ...
```

---

## 🔧 开发指南

### 添加新 UI 组件

1. **继承正确的基类**
   - 按钮 → `UGaiaButton`
   - 窗口 → `UGaiaDraggableWindow`
   - 一般控件 → `UCommonUserWidget`

2. **遵循命名规范**
   - 类名：`UGaia[功能名称]Widget`
   - 文件名：`Gaia[功能名称]Widget.h/cpp`

3. **编写文档**
   - 创建使用指南（放入 `Docs/` 对应文件夹）
   - 更新 `Docs/README.md`
   - 在代码中添加详细注释

4. **测试**
   - C++ 功能测试
   - 蓝图集成测试
   - 多平台输入测试

---

## 🐛 问题排查

### 常见问题

#### 按钮没有响应点击
检查：
- 是否调用了 `SetIsEnabled(true)`
- 是否正确绑定了 `OnClicked` 事件
- 父控件是否拦截了输入

#### 窗口不能拖动
检查：
- 标题栏的 `bIsDraggable` 是否为 `true`
- 窗口是否添加到了 Canvas Panel 中
- 标题栏是否正确绑定到窗口

#### UI 显示顺序错误
检查：
- 窗口是否在 Canvas Panel 中
- 是否调用了 `BringToFront()`
- Canvas 的子控件顺序

### 调试技巧

查看 [拖动问题调试指南](./Docs/Archive/拖动问题调试指南.md) 了解详细的调试步骤。

---

## 📚 相关资源

### Unreal Engine 文档
- [CommonUI Plugin](https://docs.unrealengine.com/5.3/en-US/common-ui-plugin-for-advanced-input-in-unreal-engine/)
- [UMG UI Designer](https://docs.unrealengine.com/5.3/en-US/umg-ui-designer-for-unreal-engine/)
- [Input System](https://docs.unrealengine.com/5.3/en-US/enhanced-input-in-unreal-engine/)

### 项目文档
- [库存系统](../Gameplay/Inventory/README.md)
- [架构说明](./架构说明.md)

---

**最后更新**: 2025-10-31  
**维护者**: Gaia 开发团队


