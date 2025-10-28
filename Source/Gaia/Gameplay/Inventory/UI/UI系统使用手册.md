# UI系统使用手册

> **快速上手指南** - 从编译到运行的完整流程

---

## 📌 目录

1. [编译项目](#编译项目)
2. [创建UMG Blueprints](#创建umg-blueprints)
3. [配置Project Settings](#配置project-settings)
4. [测试UI系统](#测试ui系统)
5. [实际使用](#实际使用)
6. [API参考](#api参考)
7. [常见问题](#常见问题)

---

## 🔧 编译项目

### 步骤

1. 打开Visual Studio
2. 编译 `GaiaGame` 项目
3. 确认编译成功（✅ 零错误、零警告）

### 检查文件

确保以下文件存在：
- `Source/Gaia/UI/GaiaUIPolicy.h/.cpp`
- `Source/Gaia/UI/GaiaPrimaryGameLayout.h/.cpp`
- `Source/Gaia/UI/Inventory/GaiaInventoryUIManager.h/.cpp`
- `Config/Tags/GameplayTags_UI.ini`

---

## 🎨 创建UMG Blueprints

### 1. WBP_GaiaPrimaryGameLayout（⭐必须）

这是整个UI系统的核心布局。

#### 创建步骤

1. **创建Widget Blueprint**
   - Content Browser → 右键 → User Interface → Widget Blueprint
   - 命名：`WBP_GaiaPrimaryGameLayout`
   - **打开Class Settings → 设置Parent Class为 `GaiaPrimaryGameLayout`**

2. **设计UI结构**
   ```
   Canvas Panel (根)
     └─ Overlay
         ├─ GameLayer (CommonActivatableWidgetStack)
         ├─ ContainerLayer (CommonActivatableWidgetStack)
         ├─ MenuLayer (CommonActivatableWidgetStack)
         └─ ModalLayer (CommonActivatableWidgetStack)
   ```

3. **添加4个Layer**
   - 在Palette中搜索 `CommonActivatableWidgetStack`
   - 分别命名为：`GameLayer`、`ContainerLayer`、`MenuLayer`、`ModalLayer`
   - **⚠️ 关键：每个都要勾选 "Is Variable"**
   - 设置Z-Order：0, 1, 2, 3
   - Anchors设置为 Fill Screen

4. **编译并保存**

### 2. BP_GaiaUIPolicy

1. **创建Blueprint Class**
   - 搜索父类：`GaiaUIPolicy`
   - 命名：`BP_GaiaUIPolicy`

2. **配置Layout**
   - 打开BP → Details面板
   - 设置 `Layout Class` 为 `WBP_GaiaPrimaryGameLayout`
   - 编译并保存

### 3. WBP_ContainerWindow（可选）

如果需要自定义容器窗口UI：
- 父类：`GaiaContainerWindowWidget`
- 绑定Widget：`Border_TitleBar`、`Text_Title`、`Button_Close`、`ContainerGrid`

---

## ⚙️ 配置Project Settings

### 方法1：编辑配置文件（推荐）

编辑 `Config/DefaultGame.ini`：

```ini
[/Script/CommonGame.GameUIManagerSubsystem]
DefaultUIPolicyClass=/Game/UI/BP_GaiaUIPolicy.BP_GaiaUIPolicy_C

[/Script/Gaia.GaiaInventoryUIManager]
ContainerWindowClass=/Game/UI/Inventory/WBP_ContainerWindow.WBP_ContainerWindow_C
```

**注意：** 路径要根据实际创建位置调整。

### 方法2：通过编辑器

1. **Project Settings → Game → Common Game**
2. 设置 `Default UI Policy Class` 为 `BP_GaiaUIPolicy`

---

## 🧪 测试UI系统

### 创建测试Actor

1. **创建Blueprint Actor** → 命名 `BP_UITestActor`

2. **Event Graph添加代码：**
   ```
   Event BeginPlay
     ↓
   Delay (1.0秒)
     ↓
   Get Gaia Inventory UI Manager (WorldContext = Self)
     ↓
   Print String: "UI管理器已初始化！"
   ```

3. **放入场景并Play**

4. **查看日志：**
   ```
   LogGaia: [主布局] 初始化完成，已注册所有Layer
   LogGaia: [库存UI管理器] 初始化
   ```

---

## 💻 实际使用

### C++示例

```cpp
// 打开容器窗口
void OpenContainer(FGuid ContainerUID)
{
    UGaiaInventoryUIManager* UIManager = UGaiaInventoryUIManager::Get(this);
    if (UIManager)
    {
        UIManager->OpenContainerWindow(ContainerUID);
    }
}

// 关闭容器窗口
void CloseContainer(FGuid ContainerUID)
{
    UGaiaInventoryUIManager* UIManager = UGaiaInventoryUIManager::Get(this);
    if (UIManager && UIManager->IsContainerWindowOpen(ContainerUID))
    {
        UIManager->CloseAllContainerWindows();
    }
}
```

### Blueprint示例

**打开容器：**
```
Get Gaia Inventory UI Manager
  → Open Container Window
      Container UID: [容器UID]
```

**关闭容器：**
```
Get Gaia Inventory UI Manager
  → Close All Container Windows
```

**注意：** ESC键会自动由Layer System处理，无需手动绑定！

---

## 📚 API参考

### UIManager常用函数

| 函数 | 说明 |
|------|------|
| `Get(WorldContext)` | 获取UI管理器实例 |
| `OpenContainerWindow(UID)` | 打开容器窗口 |
| `CloseContainerWindow(Widget)` | 关闭指定窗口 |
| `CloseAllContainerWindows()` | 关闭所有窗口 |
| `IsContainerWindowOpen(UID)` | 检查窗口是否打开 |

### PrimaryGameLayout Tags

| Tag | 用途 |
|-----|------|
| `UI.Layer.Inventory.Game` | 玩家背包、装备栏 |
| `UI.Layer.Inventory.Container` | 容器窗口（可多开） |
| `UI.Layer.Inventory.Menu` | 右键菜单、Tooltip |
| `UI.Layer.Inventory.Modal` | 模态对话框 |

---

## 🐛 常见问题

### Q1: 编译错误 - 找不到CommonUI类

**解决：** 在 `YourProject.Build.cs` 中添加：
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "CommonUI", "CommonGame", "GameplayTags"
});
```

### Q2: PIE后看不到初始化日志

**原因：** UIPolicy未配置  
**解决：** 检查 `DefaultGame.ini` 配置是否正确

### Q3: 窗口打开了但看不见

**原因：** Widget蓝图内容为空  
**解决：** 确认 `WBP_ContainerWindow` 有可见内容

### Q4: ESC键不工作

**原因：** Layer System未正确配置  
**解决：** 
- 确认所有Layer都是 `CommonActivatableWidgetStack` 类型
- 确认Z-Order设置正确
- 确认变量名完全匹配（区分大小写）

### Q5: 多个窗口Z-Order混乱

**解决：** 确认所有容器窗口都推入 `ContainerLayer`

---

## 🎓 最佳实践

### 1. 始终检查返回值

```cpp
UGaiaInventoryUIManager* UIManager = UGaiaInventoryUIManager::Get(this);
if (!UIManager)
{
    UE_LOG(LogTemp, Error, TEXT("UI管理器未初始化"));
    return;
}
```

### 2. 使用正确的Layer

- **Game Layer** → 主要游戏UI
- **Container Layer** → 可多开的容器窗口
- **Menu Layer** → 右键菜单、Tooltip
- **Modal Layer** → 模态对话框

### 3. 让Layer System管理生命周期

```cpp
// ✅ 正确
void CloseWindow() {
    DeactivateWidget();
}

// ❌ 错误
void CloseWindow() {
    RemoveFromParent(); // 绕过Layer System
}
```

---

## 🔍 调试技巧

### 启用调试日志

在 `Output Log` 中过滤 `LogGaia`：
- **Log** - 一般信息
- **Verbose** - 详细调试
- **Warning** - 警告
- **Error** - 错误

### 控制台命令

```
# 显示所有容器
Gaia.InventoryDebug.DumpContainers

# 显示所有物品
Gaia.InventoryDebug.DumpItems
```

### 启用调试模式

```cpp
ContainerWindow->SetDebugMode(true);
```

---

## 📖 相关文档

- [UI系统架构说明.md](UI系统架构说明.md) - 深入了解系统设计
- [README.md](README.md) - UI系统概览
- [../README.md](../README.md) - 库存系统主文档

---

## 🎯 快速检查清单

创建UI系统前的检查：

- [ ] ✅ 编译成功，无错误
- [ ] ✅ 创建了 `WBP_GaiaPrimaryGameLayout`
- [ ] ✅ 4个Layer都正确绑定（Is Variable已勾选）
- [ ] ✅ 创建了 `BP_GaiaUIPolicy`
- [ ] ✅ UIPolicy的Layout Class已设置
- [ ] ✅ `DefaultGame.ini` 已配置
- [ ] ✅ PIE后能看到初始化日志

---

**当前状态：** 核心功能完成，可以使用！

**遇到问题？** 查看 [常见问题](#常见问题) 或检查日志输出。

