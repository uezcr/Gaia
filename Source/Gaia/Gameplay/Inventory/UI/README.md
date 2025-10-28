# 库存UI系统文档

> **基于CommonUI的现代化库存UI系统**  
> **状态：** ✅ 核心功能完成，可以使用  
> **编译状态：** ✅ 零错误、零警告

---

## 📚 文档导航

### 核心文档（必读）

1. **[UI系统使用手册.md](UI系统使用手册.md)** - 完整的使用指南，从编译到运行
2. **[UI系统架构说明.md](UI系统架构说明.md)** - 系统架构和设计理念

---

## 🎯 快速开始

### 3步开始使用

1. **编译项目**
   ```bash
   # 在UE编辑器中编译，确保无错误
   ```

2. **创建UMG Blueprints**
   - WBP_GaiaPrimaryGameLayout（必须）
   - WBP_ContainerWindow（容器窗口）
   - WBP_ItemSlot（物品槽位）
   - 详见 [UI系统使用手册.md](UI系统使用手册.md)

3. **测试功能**
   ```cpp
   // 打开容器窗口
   UGaiaInventoryUIManager* UIManager = UGaiaInventoryUIManager::Get(GetWorld());
   UIManager->OpenContainerWindow(ContainerUID);
   ```

---

## 🏗️ 系统架构

### 核心组件

```
UGaiaInventoryUIManager (UI管理器)
    ↓
UGaiaPrimaryGameLayout (主布局)
    ↓
4个Layer (UI层级)
    ├─ GameLayer (玩家背包等)
    ├─ ContainerLayer (容器窗口)
    ├─ MenuLayer (右键菜单)
    └─ ModalLayer (对话框)
        ↓
UGaiaContainerWindowWidget (容器窗口)
    ↓
UGaiaContainerGridWidget (网格)
    ↓
UGaiaItemSlotWidget (槽位)
```

### 技术栈

- **UE5 CommonUI Plugin** - 完整采用CommonUI架构
- **GameplayTag System** - UI层级管理
- **Event-Driven** - RPC事件驱动UI更新
- **Network-Ready** - 完整的网络支持

---

## ✨ 核心功能

### 已实现功能

| 功能 | 状态 | 说明 |
|------|------|------|
| 物品槽位显示 | ✅ | 图标、数量、高亮 |
| 拖放操作 | ✅ | 移动、交换、堆叠 |
| 容器窗口 | ✅ | 多窗口、关闭、调试信息 |
| 网络同步 | ✅ | 自动同步服务器数据 |
| 权限检查 | ✅ | 自动检查容器权限 |
| Layer管理 | ✅ | 4层UI互不干扰 |

### 可选功能（待实现）

| 功能 | 优先级 | 说明 |
|------|--------|------|
| 右键菜单 | 高 | 使用、丢弃等操作 |
| 数量输入对话框 | 高 | 拆分物品时输入数量 |
| 物品Tooltip | 中 | 悬停显示详细信息 |
| 拖放视觉反馈 | 中 | 拖放时的视觉Widget |

---

## 📋 代码文件清单

### C++代码

| 文件 | 说明 |
|------|------|
| `Source/Gaia/UI/GaiaUIPolicy.h/.cpp` | UI策略 |
| `Source/Gaia/UI/GaiaPrimaryGameLayout.h/.cpp` | 主布局 |
| `Source/Gaia/UI/Inventory/GaiaInventoryUIManager.h/.cpp` | UI管理器 |
| `Source/Gaia/UI/Inventory/GaiaContainerWindowWidget.h/.cpp` | 容器窗口 |
| `Source/Gaia/UI/Inventory/GaiaContainerGridWidget.h/.cpp` | 容器网格 |
| `Source/Gaia/UI/Inventory/GaiaItemSlotWidget.h/.cpp` | 物品槽位 |
| `Source/Gaia/UI/Inventory/GaiaItemDragDropOperation.h/.cpp` | 拖放操作 |
| `Source/Gaia/UI/Inventory/GaiaContainerDebugInfoWidget.h/.cpp` | 调试信息 |

### 配置文件

| 文件 | 说明 |
|------|------|
| `Config/Tags/GameplayTags_UI.ini` | UI层级Tag定义 |

---

## 🎨 UMG Blueprints清单

需要创建以下Blueprints：

| Blueprint | 父类 | 优先级 | 说明 |
|-----------|------|--------|------|
| `WBP_GaiaPrimaryGameLayout` | `GaiaPrimaryGameLayout` | ⭐ 必须 | 主UI布局 |
| `WBP_ContainerWindow` | `GaiaContainerWindowWidget` | 高 | 容器窗口 |
| `WBP_ContainerGrid` | `GaiaContainerGridWidget` | 高 | 容器网格 |
| `WBP_ItemSlot` | `GaiaItemSlotWidget` | 高 | 物品槽位 |
| `WBP_ContainerDebugInfo` | `GaiaContainerDebugInfoWidget` | 中 | 调试信息（可选） |

详细创建步骤见 [UI系统使用手册.md](UI系统使用手册.md)

---

## 🔧 Project Settings配置

### 1. GameplayTag设置

**Project Settings → GameplayTags**
- ✅ 添加 `Config/Tags/GameplayTags_UI.ini` 到搜索路径

### 2. UIPolicy设置

**Project Settings → Plugins → Game UI Manager**
- ✅ 创建 `BP_GaiaUIPolicy`（父类：GaiaUIPolicy）
- ✅ 设置 `LayoutClass` 为 `WBP_GaiaPrimaryGameLayout`
- ✅ 在Project Settings中设置 `Default UI Policy Class` 为 `BP_GaiaUIPolicy`

---

## 💡 使用示例

### Blueprint使用

```
# 打开容器窗口
Get Gaia Inventory UI Manager
    → Open Container Window
        ContainerUID: [容器UID]
        bSuspendInputUntilComplete: false

# 关闭容器窗口
Get Gaia Inventory UI Manager
    → Close Container Window
        Widget: [窗口引用]
```

### C++使用

```cpp
// 获取UI管理器
UGaiaInventoryUIManager* UIManager = UGaiaInventoryUIManager::Get(GetWorld());

// 打开容器窗口
UGaiaContainerWindowWidget* Window = UIManager->OpenContainerWindow(ContainerUID);

// 关闭容器窗口
UIManager->CloseContainerWindow(Window);

// 关闭所有容器窗口
UIManager->CloseAllContainerWindows();

// 检查窗口是否打开
bool bIsOpen = UIManager->IsContainerWindowOpen(ContainerUID);
```

---

## 🐛 调试技巧

### 启用调试信息

```cpp
// 在容器窗口上启用调试模式
ContainerWindow->SetDebugMode(true);
```

调试信息包括：
- 容器UID和定义ID
- 所有权类型和所有者
- 授权玩家列表
- 当前访问者
- 槽位使用情况
- 物品列表

### 控制台命令

```
# 显示所有容器信息
Gaia.InventoryDebug.DumpContainers

# 显示所有物品信息
Gaia.InventoryDebug.DumpItems
```

### 日志查看

在 `Output Log` 中过滤 `LogGaia` 分类：
- **Log** - 一般信息
- **Verbose** - 详细调试信息
- **Warning** - 警告
- **Error** - 错误

---

## ⚠️ 常见问题

### Q1: 编译错误 - 找不到CommonUI类

**解决：** 在 `YourProject.Build.cs` 中添加依赖：
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "CommonUI",
    "CommonGame",
    "GameplayTags"
});
```

### Q2: Layer没有注册

**解决：** 检查 `WBP_GaiaPrimaryGameLayout` 中：
- 变量名是否正确（GameLayer, ContainerLayer等）
- 是否勾选了 **Is Variable**
- 类型是否为 `CommonActivatableWidgetStack`

### Q3: 窗口打不开

**解决：** 检查：
- UIPolicy是否正确配置
- PrimaryGameLayout是否正确设置
- ContainerWindowClass是否在Project Settings中配置

### Q4: 拖放不工作

**解决：** 检查：
- 槽位Widget是否正确绑定到网格
- ItemSlotWidgetClass是否在ContainerGrid中设置
- RPC组件是否正确添加到PlayerController

---

## 📖 扩展阅读

### 官方文档
- [UE5 CommonUI文档](https://docs.unrealengine.com/5.0/en-US/common-ui-plugin-for-advanced-input-in-unreal-engine/)
- [GameplayTag系统](https://docs.unrealengine.com/5.0/en-US/gameplay-tags-in-unreal-engine/)

### 相关系统文档
- [库存系统核心文档](../README.md)
- [网络RPC组件](../RPCComponent使用指南.md)
- [容器权限系统](../容器所有权和权限系统.md)

---

## 🎯 下一步

1. ✅ **编译通过** - 代码已准备好
2. ⏳ **创建Blueprints** - 按使用手册创建UMG
3. ⏳ **配置Settings** - 设置UIPolicy等
4. ⏳ **测试功能** - 打开窗口、拖放物品
5. ⏳ **实现增强功能** - 右键菜单、Tooltip等

---

**当前状态：** 核心UI系统已完成，可以开始使用！

**如有问题：** 请查看 [UI系统使用手册.md](UI系统使用手册.md) 或提Issue

