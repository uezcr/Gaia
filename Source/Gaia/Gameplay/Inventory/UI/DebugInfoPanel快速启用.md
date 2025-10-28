# DebugInfoPanel 快速启用

> 5分钟快速启用调试面板

---

## 🚀 快速步骤

### 1️⃣ 创建 DebugInfo Widget（2分钟）

```
1. Content Browser → 右键 → User Interface → Widget Blueprint
2. 名称: WBP_ContainerDebugInfo
3. 父类: GaiaContainerDebugInfoWidget
```

**添加组件：**
```
Vertical Box
├─ TextBlock (Text_ContainerUID)
├─ TextBlock (Text_OwnershipType)
├─ TextBlock (Text_Owner)
├─ TextBlock (Text_AuthorizedPlayers)
├─ TextBlock (Text_CurrentAccessor)
├─ TextBlock (Text_SlotUsage)
└─ ScrollBox (ScrollBox_ItemList)
```

**重要：**
- ✅ 勾选所有组件的 `Is Variable`
- ✅ 组件名称必须完全匹配（包括下划线）

---

### 2️⃣ 添加到 ContainerWindow（1分钟）

```
1. 打开 WBP_GaiaContainerWindow
2. 拖入 WBP_ContainerDebugInfo
3. 重命名为: DebugInfoPanel
4. ✅ 勾选 Is Variable
5. 设置 Visibility: Collapsed
```

---

### 3️⃣ 启用调试模式（2分钟）

**方式A：蓝图**
```blueprint
WBP_GaiaContainerWindow → Event Construct
  → SetDebugMode(true)
```

**方式B：C++**
```cpp
// GaiaUIManagerSubsystem.cpp
void UGaiaUIManagerSubsystem::OpenContainerWindow(const FGuid& ContainerUID)
{
    // ... 创建窗口后 ...
    
#if !UE_BUILD_SHIPPING
    ContainerWindow->SetDebugMode(true);
#endif
}
```

---

## ✅ 完成！

运行游戏，打开容器，应该能看到调试信息。

**特性：**
- ✅ 事件驱动实时刷新（与槽位同步）
- ✅ 字体大小由UMG控制（代码不设置）
- ✅ 零延迟、零开销（只在库存变化时刷新）

---

## ❌ 不显示？快速排查

### 检查1：组件名称
```
UMG: Text_ContainerUID  ← 必须完全一致
C++:  Text_ContainerUID
```

### 检查2：Is Variable
所有组件都要勾选 `Is Variable`

### 检查3：SetDebugMode
确认调用了 `SetDebugMode(true)`

### 检查4：编译
重新编译项目

---

## 📄 完整文档

详细说明见：[DebugInfoPanel使用指南.md](DebugInfoPanel使用指南.md)

