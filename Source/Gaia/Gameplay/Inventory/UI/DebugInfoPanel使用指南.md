# DebugInfoPanel 使用指南

## 📋 概述

`DebugInfoPanel` 是容器窗口的调试信息面板，用于在开发和测试阶段显示容器的详细信息。

---

## 🎯 功能特性

显示以下调试信息：
- ✅ 容器UID和定义ID
- ✅ 所有权类型（私有/世界/共享/交易）
- ✅ 所有者信息
- ✅ 授权玩家列表
- ✅ 当前访问者
- ✅ 槽位使用情况（已用/总数）
- ✅ 重量和体积信息
- ✅ 物品列表（详细信息）
- ✅ **事件驱动实时刷新**（库存变化时自动更新）

---

## 🔧 设置步骤

### 步骤1：创建 DebugInfoPanel UMG Widget

1. **创建新Widget蓝图**
   - 路径：`Content/UI/Inventory/`
   - 名称：`WBP_ContainerDebugInfo`
   - 父类：`GaiaContainerDebugInfoWidget`（C++类）

2. **添加必要的UI组件**

```
Canvas Panel (Root)
└─ Border (Border_DebugPanel) ← 可选的背景边框
   └─ Vertical Box
      ├─ TextBlock (Text_ContainerUID)        ← 容器UID
      ├─ TextBlock (Text_OwnershipType)       ← 所有权类型
      ├─ TextBlock (Text_Owner)               ← 所有者
      ├─ TextBlock (Text_AuthorizedPlayers)   ← 授权玩家
      ├─ TextBlock (Text_CurrentAccessor)     ← 当前访问者
      ├─ TextBlock (Text_SlotUsage)           ← 槽位使用情况
      └─ ScrollBox (ScrollBox_ItemList)       ← 物品列表
```

3. **设置组件属性**

**所有 TextBlock 组件：**
- `Is Variable`: ✅ 勾选
- `Auto Wrap Text`: ✅ 勾选（推荐）
- `Font Size`: 12-14
- `Color`: 白色或浅灰色

**ScrollBox_ItemList：**
- `Is Variable`: ✅ 勾选
- `Orientation`: Vertical
- `Max Height`: 200-300（根据需要）

**Border_DebugPanel（可选）：**
- `Is Variable`: ✅ 勾选
- `Brush Color`: 半透明黑色 (0, 0, 0, 0.8)
- `Padding`: 10, 10, 10, 10

4. **确保组件命名正确**

⚠️ **重要**：组件名称**必须**与C++中的变量名完全匹配：

| UMG组件名 | C++变量名 | 是否必需 |
|-----------|----------|---------|
| `Text_ContainerUID` | `Text_ContainerUID` | 可选 |
| `Text_OwnershipType` | `Text_OwnershipType` | 可选 |
| `Text_Owner` | `Text_Owner` | 可选 |
| `Text_AuthorizedPlayers` | `Text_AuthorizedPlayers` | 可选 |
| `Text_CurrentAccessor` | `Text_CurrentAccessor` | 可选 |
| `Text_SlotUsage` | `Text_SlotUsage` | 可选 |
| `ScrollBox_ItemList` | `ScrollBox_ItemList` | 可选 |
| `Border_DebugPanel` | `Border_DebugPanel` | 可选 |

**注意**：所有组件都是 `BindWidgetOptional`，不是必需的，但建议全部添加。

---

### 步骤2：在 ContainerWindow 中添加 DebugInfoPanel

1. **打开容器窗口Widget**
   - 打开 `WBP_GaiaContainerWindow`

2. **添加 DebugInfoPanel**

```
Canvas Panel (Root)
├─ Border (主窗口)
│  ├─ Title Bar
│  ├─ ContainerGrid
│  └─ ...
└─ WBP_ContainerDebugInfo (DebugInfoPanel) ← 添加到这里
```

3. **设置 DebugInfoPanel 属性**

- **组件名称**：`DebugInfoPanel`（必须）
- **Is Variable**：✅ 勾选
- **位置**：放在窗口右侧或下方
- **大小**：根据需要调整
- **初始可见性**：`Collapsed`（默认隐藏）

4. **布局建议**

**方案A：右侧布局**
```
Horizontal Box
├─ Container Grid (70%)
└─ Debug Panel (30%)
```

**方案B：下方布局**
```
Vertical Box
├─ Container Grid (70%)
└─ Debug Panel (30%)
```

---

### 步骤3：启用调试模式

调试模式默认关闭，需要手动启用。

#### 方式1：在蓝图中启用

**Event Graph:**
```blueprint
Event Construct
  → Call SetDebugMode(true)
```

或者添加一个切换按钮：
```blueprint
Button_ToggleDebug OnClicked
  → Branch
     ├─ True → SetDebugMode(false)
     └─ False → SetDebugMode(true)
```

---

#### 方式2：在C++中启用

**打开容器时启用调试模式：**

```cpp
// GaiaUIManagerSubsystem.cpp
void UGaiaUIManagerSubsystem::OpenContainerWindow(const FGuid& ContainerUID)
{
    // ... 创建窗口 ...
    
    // 启用调试模式（仅在开发构建中）
#if !UE_BUILD_SHIPPING
    ContainerWindow->SetDebugMode(true);
#endif
    
    // ... 其余代码 ...
}
```

---

#### 方式3：通过控制台命令启用

创建一个控制台命令来切换调试模式：

**新增C++代码（可选）：**

```cpp
// GaiaContainerWindowWidget.h
#if !UE_BUILD_SHIPPING
public:
    /**
     * 控制台命令：切换调试模式
     */
    UFUNCTION(Exec, Category = "Gaia|Debug")
    static void GaiaInventoryToggleDebug();
#endif
```

```cpp
// GaiaContainerWindowWidget.cpp
#if !UE_BUILD_SHIPPING
void UGaiaContainerWindowWidget::GaiaInventoryToggleDebug()
{
    // 遍历所有打开的容器窗口，切换调试模式
    if (UGaiaUIManagerSubsystem* UIMgr = UGaiaUIManagerSubsystem::Get(GWorld))
    {
        // 需要添加 GetOpenContainerWindows() 函数
        // ...
    }
}
#endif
```

---

## 🧪 测试步骤

### 1. 编译项目
```bash
# 确保没有编译错误
Build Project
```

### 2. 打开编辑器

### 3. 验证 DebugInfoPanel 创建

- 打开 `WBP_ContainerDebugInfo`
- 检查所有组件是否正确命名
- 检查父类是否为 `GaiaContainerDebugInfoWidget`

### 4. 验证 ContainerWindow 集成

- 打开 `WBP_GaiaContainerWindow`
- 检查 `DebugInfoPanel` 组件是否存在
- 检查 `Is Variable` 是否勾选

### 5. 运行游戏测试

```cpp
// 创建容器
FGuid ContainerUID = CreateContainerForPlayer("TestContainer", PlayerController);

// 添加物品
FGaiaItemInstance Item = CreateItemInstance("TestItem", 5);
AddItemToContainer(Item, ContainerUID, 0);

// 打开容器窗口（调试模式应该已启用）
UGaiaUIManagerSubsystem* UIMgr = UGaiaUIManagerSubsystem::Get(World);
UIMgr->OpenContainerWindow(ContainerUID);
```

### 6. 预期结果

打开容器窗口后，应该看到：

```
容器UID: 7B9293814D119E27802F07BFEF4F6468
定义ID: TestContainer

所有权类型: 私有

所有者: BP_GaiaPlayerController_C_0

授权玩家:
无

当前访问者: 无

槽位使用: 1/10
体积: 5/100

物品列表:
- 槽位0: TestItem x5 (UID: ...)
```

---

## 🎨 UI样式建议

### 推荐样式

```
Background: 半透明黑色 (0, 0, 0, 0.8)
Border: 浅灰色边框 (0.2, 0.2, 0.2, 1.0)
Text Color: 白色或浅灰色
Font Size: 12-14

Title (容器UID): 加粗, 略大 (16)
Labels (所有者、类型等): 正常 (12)
Item List: 等宽字体（推荐）
```

### CSS风格参考

```css
.debug-panel {
    background: rgba(0, 0, 0, 0.8);
    border: 1px solid #333;
    padding: 10px;
    margin: 10px;
}

.debug-title {
    color: #FFF;
    font-size: 16px;
    font-weight: bold;
    margin-bottom: 8px;
}

.debug-label {
    color: #CCC;
    font-size: 12px;
    margin: 4px 0;
}

.debug-item-list {
    color: #FFF;
    font-family: monospace;
    font-size: 11px;
}
```

---

## 🔍 常见问题

### Q1: DebugInfoPanel 不显示

**原因1：未调用 SetDebugMode(true)**
```cpp
// 解决：在打开窗口时调用
ContainerWindow->SetDebugMode(true);
```

**原因2：组件未绑定**
- 检查 UMG 中的组件名称是否正确
- 检查 `Is Variable` 是否勾选

**原因3：Visibility 设置错误**
- 检查初始可见性是否为 `Collapsed`
- `SetDebugMode(true)` 会自动设置为 `Visible`

---

### Q2: 组件显示为空

**原因：组件名称不匹配**

检查 C++ 代码中的变量名：
```cpp
UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
TObjectPtr<UTextBlock> Text_ContainerUID;  // ← 这个名字
```

必须与 UMG 中的组件名完全一致：
```
TextBlock → Name: Text_ContainerUID  // ← 必须一致
```

---

### Q3: 物品列表不显示

**检查 ScrollBox**

1. 组件名称：`ScrollBox_ItemList`
2. `Is Variable`: ✅ 勾选
3. 方向：`Vertical`

---

### Q4: 如何只在开发版本中显示

```cpp
// 在 OpenContainerWindow 中
#if !UE_BUILD_SHIPPING
    ContainerWindow->SetDebugMode(true);
#else
    ContainerWindow->SetDebugMode(false);
#endif
```

---

## 📊 调试信息说明

### 容器UID和定义ID
```
容器UID: 7B9293814D119E27802F07BFEF4F6468  ← 运行时唯一标识
定义ID: TestContainer                      ← 数据表中的定义
```

### 所有权类型
- **私有**：只有所有者可访问（玩家背包）
- **世界**：任何人可访问，独占（宝箱）
- **共享**：授权玩家可访问（公会仓库）
- **交易**：临时交易容器

### 所有者
- 显示容器的拥有者名称
- 如果无所有者，显示"无"

### 授权玩家
- 仅对共享容器有意义
- 显示所有被授权访问的玩家列表

### 当前访问者
- 仅对世界容器有意义
- 显示当前正在访问的玩家

### 槽位使用情况
```
槽位使用: 3/10      ← 已使用3个槽位，总共10个
体积: 25/100        ← 已使用25点体积，总共100点
```

### 物品列表
```
- 槽位0: Wood x50 (UID: ABC...)
- 槽位1: IronSword x1 (UID: DEF...)
- 槽位5: LeatherBackpack x1 (容器: GHI...)
```

---

## 🚀 进阶功能

### 事件驱动刷新（已内置）✅

系统使用**事件驱动**机制，与槽位刷新完全相同，无需定时器轮询。

**实现细节：**
```
库存变化（物品移动/添加/删除）
  ↓
RPC组件广播 OnInventoryUpdated 事件
  ↓
UIManagerSubsystem::OnInventoryUpdated()
  ├─ 刷新所有槽位（RefreshAllSlots）
  └─ 刷新调试面板（RefreshDebugInfo）  ← 同步刷新
```

**优势：**
- ✅ 零延迟：库存变化立即反映
- ✅ 零开销：只在变化时刷新，不轮询
- ✅ 与槽位同步：完全一致的刷新时机

**代码实现：**
```cpp
// GaiaUIManagerSubsystem.cpp
void UGaiaUIManagerSubsystem::OnInventoryUpdated()
{
    for (auto& Pair : OpenContainerWindows)
    {
        if (UGaiaContainerWindowWidget* Window = Pair.Value)
        {
            // 刷新容器网格
            Window->GetContainerGrid()->RefreshAllSlots();
            
            // 刷新调试信息（事件驱动，同步刷新）
            Window->RefreshDebugInfo();
        }
    }
}
```

### 添加切换按钮

```blueprint
Button_ToggleDebug OnClicked
  → Branch
     Condition: bDebugMode
     ├─ True → SetDebugMode(false)
     └─ False → SetDebugMode(true)
```

### 手动刷新按钮

如果需要手动刷新按钮：

```blueprint
Button_RefreshDebug OnClicked
  → Call RefreshDebugInfo()
```

---

## ✅ 检查清单

在启用 DebugInfoPanel 前，确认：

- ✅ 创建了 `WBP_ContainerDebugInfo` Widget
- ✅ 设置了父类为 `GaiaContainerDebugInfoWidget`
- ✅ 添加了所有必需的组件（TextBlock、ScrollBox等）
- ✅ 组件名称与C++变量名完全一致
- ✅ 所有组件的 `Is Variable` 都已勾选
- ✅ 在 `WBP_GaiaContainerWindow` 中添加了 `DebugInfoPanel`
- ✅ `DebugInfoPanel` 的组件名正确
- ✅ 调用了 `SetDebugMode(true)` 启用调试模式
- ✅ 编译项目成功
- ✅ 测试显示正常

---

## 📝 相关文档

- [UI系统使用手册.md](UI系统使用手册.md) - 完整的UI系统指南
- [UI系统架构说明.md](UI系统架构说明.md) - 架构设计说明
- [UI故障排查指南.md](UI故障排查指南.md) - 问题排查

---

## 💡 最佳实践

1. **仅在开发版本中启用**
   ```cpp
   #if !UE_BUILD_SHIPPING
       ContainerWindow->SetDebugMode(true);
   #endif
   ```

2. **提供切换选项**
   - 添加快捷键（如 F3）切换调试模式
   - 或在设置菜单中添加选项

3. **定期刷新**
   - 在 `NativeOnActivated` 中自动刷新
   - 或使用定时器定期更新

4. **样式一致**
   - 使用项目统一的调试UI样式
   - 保持字体、颜色、布局一致

5. **性能考虑**
   - 调试模式默认关闭
   - 避免在Shipping版本中包含调试代码

---

**现在可以开始设置 DebugInfoPanel 了！** 🎉

