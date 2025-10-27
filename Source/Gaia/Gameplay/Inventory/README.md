# Gaia 库存系统文档

## 📚 **文档索引**

### 核心文档

1. **[系统设计说明.md](系统设计说明.md)** - 库存系统的整体架构和设计理念
2. **[执行流程说明.md](执行流程说明.md)** - 详细的操作执行流程和数据流转
3. **[测试指南.md](测试指南.md)** - 如何测试库存系统

### 测试相关

4. **[移动功能测试计划.md](移动功能测试计划.md)** - 物品移动功能的详细测试计划（12个测试用例）
5. **[测试Actor使用指南.md](测试Actor使用指南.md)** - 测试Actor的配置和使用说明

### 联机功能

6. **[联机同步方案.md](联机同步方案.md)** - 多人游戏网络同步方案（完整方案对比）
7. **[基于WorldSubsystem的联机方案.md](基于WorldSubsystem的联机方案.md)** - 推荐的简化联机方案（核心）
8. **[RPCComponent使用指南.md](RPCComponent使用指南.md)** - RPC组件的使用说明和示例代码
9. **[容器所有权和权限系统.md](容器所有权和权限系统.md)** - 完整的权限控制系统（Private/World/Shared/Trade容器）⭐
10. **[容器独占访问功能说明.md](容器独占访问功能说明.md)** - 世界容器独占访问机制（防止多人同时打开）
11. **[服务器广播功能说明.md](服务器广播功能说明.md)** - 智能广播系统，自动同步容器更新

---

## 🎯 **快速开始**

### 1. 了解系统设计

阅读 **[系统设计说明.md](系统设计说明.md)** 了解：
- 物品和容器的数据结构
- 物品位置自存储架构
- 容器嵌套机制
- GUID 唯一性管理

### 2. 理解执行流程

阅读 **[执行流程说明.md](执行流程说明.md)** 了解：
- 每个操作的详细步骤
- 数据如何在系统中流转
- 函数调用顺序
- 各种边界情况的处理

### 3. 测试系统

阅读 **[测试指南.md](测试指南.md)** 了解：
- 如何运行测试
- 如何创建测试关卡
- 如何查看测试结果

---

## 🔑 **核心概念**

### 物品实例 (FGaiaItemInstance)

每个物品实例具有：
- **唯一ID (FGuid)**：全局唯一标识符
- **位置信息**：自己存储当前所在容器和槽位
- **数量**：支持堆叠
- **拥有容器**：如果是背包类物品，可以有自己的容器

### 容器实例 (FGaiaContainerInstance)

每个容器实例具有：
- **唯一ID (FGuid)**：全局唯一标识符
- **槽位列表**：只存储物品UID的引用
- **拥有者物品**：如果属于某个物品（如背包）
- **父容器**：如果嵌套在另一个容器中

### 库存子系统 (UGaiaInventorySubsystem)

全局管理器，维护：
- **AllItems**：所有物品实例的映射表（单一数据源）
- **Containers**：所有容器实例的映射表

---

## 🏗️ **架构特点**

### ✅ 物品位置自存储

物品自己知道自己在哪里：
```cpp
FGaiaItemInstance Item;
Item.CurrentContainerUID;  // 所在容器
Item.CurrentSlotID;        // 所在槽位
Item.IsInContainer();      // 是否在容器中
Item.IsOrphan();           // 是否游离状态
```

### ✅ 单一数据源

物品数据只存储在一个地方：
- `AllItems` 是唯一的物品数据存储
- 容器只存储物品UID的引用
- 没有数据冗余，不会出现不一致

### ✅ 容器嵌套

支持背包嵌套背包：
- 体积约束：子容器总体积不能超过父容器
- 循环检测：防止A包含B，B又包含A
- 递归删除：删除背包时，自动删除其中的所有物品

### ✅ GUID 唯一性

每个物品和容器都有全局唯一ID：
- 使用 `FGuid::NewGuid()` 生成
- 支持存档/读档
- 支持网络同步

---

## 🎮 **使用示例**

### 创建容器

```cpp
UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContext);

// 创建玩家背包
FGuid BackpackUID = InventorySystem->CreateContainer(TEXT("PlayerBackpack"));
```

### 创建物品

```cpp
// 创建10个木头
FGaiaItemInstance Wood = InventorySystem->CreateItemInstance(TEXT("Wood"), 10);

// 创建1个背包物品（自带容器）
FGaiaItemInstance Backpack = InventorySystem->CreateItemInstance(TEXT("Backpack_Small"), 1);
```

### 添加物品到容器

```cpp
// 添加木头到背包
InventorySystem->AddItemToContainer(Wood, BackpackUID);

// 添加子背包到主背包
InventorySystem->AddItemToContainer(Backpack, BackpackUID);
```

### 移动物品

```cpp
// 移动物品到指定槽位
FMoveItemResult Result = InventorySystem->MoveItem(
    ItemUID,           // 要移动的物品
    TargetContainerUID, // 目标容器
    TargetSlotID,      // 目标槽位
    Quantity           // 移动数量
);

if (Result.IsSuccess())
{
    // 移动成功
}
```

### 查询物品

```cpp
// 通过UID查找物品
FGaiaItemInstance Item;
if (InventorySystem->FindItemByUID(ItemUID, Item))
{
    // 找到物品
    UE_LOG(LogTemp, Log, TEXT("物品: %s, 数量: %d"), 
        *Item.ItemDefinitionID.ToString(), Item.Quantity);
}

// 获取容器中的所有物品
TArray<FGaiaItemInstance> Items = InventorySystem->GetItemsInContainer(ContainerUID);

// 获取游离物品（掉落在地上、装备在身上等）
TArray<FGaiaItemInstance> OrphanItems = InventorySystem->GetOrphanItems();

// 统计指定类型物品的总数量
int32 TotalWood = InventorySystem->CountItemsByType(TEXT("Wood"));
```

### 删除物品

```cpp
// 从容器移除（设为游离状态）
InventorySystem->RemoveItemFromContainer(ItemUID);

// 完全删除物品
InventorySystem->DestroyItem(ItemUID);
```

---

## 🐛 **调试技巧**

### 1. 使用控制台命令

运行游戏后，按 `~` 键打开控制台：

```bash
# 查看所有容器
Gaia.Inventory.PrintAllContainers

# 查看所有物品
Gaia.Inventory.PrintAllItems
```

### 2. 运行测试

在测试关卡中放置 `AGaiaInventoryTestActor`：
- BeginPlay 时自动运行测试
- 测试结果输出到日志

### 3. 检查日志

日志位置：
- Output Log 窗口（UE编辑器）
- `Saved/Logs/Gaia.log` 文件

过滤日志：
- 输入 `LogGaia` 只显示库存系统日志

### 4. 数据验证

```cpp
// 验证数据一致性
bool bIsValid = InventorySystem->ValidateDataIntegrity();

// 修复数据不一致
InventorySystem->RepairDataIntegrity();
```

---

## ⚙️ **系统配置**

### 物品定义 (DataRegistry)

在 `Content/Data/Registry/DR_GaiaItem.uasset` 中定义：

```
Wood:
  DisplayName: 木头
  Description: 基础建筑材料
  Weight: 2
  Volume: 5
  MaxStackSize: 99
  
Backpack_Small:
  DisplayName: 小背包
  Description: 增加10个槽位
  Weight: 10
  Volume: 15
  MaxStackSize: 1
  ContainerDefinitionID: Container_Backpack_Small
```

### 容器定义 (DataRegistry)

在 `Content/Data/Registry/DR_GaiaContainer.uasset` 中定义：

```
PlayerBackpack:
  DisplayName: 玩家背包
  SlotCount: 20
  MaxVolume: 500
  bEnableVolumeLimit: true
  
Container_Backpack_Small:
  DisplayName: 小背包容器
  SlotCount: 10
  MaxVolume: 150
  bEnableVolumeLimit: true
```

---

## 📈 **性能特点**

### O(1) 查找

- 通过UID查找物品：`AllItems.Find(UID)` - O(1)
- 通过UID查找容器：`Containers.Find(UID)` - O(1)

### 最小化数据冗余

- 物品数据只存在 `AllItems` 中
- 容器槽位只存储UID引用
- 没有重复存储，内存高效

### 快速移动

- 容器内移动：只需修改物品的 `CurrentSlotID`
- 跨容器移动：只需修改物品的 `CurrentContainerUID` 和 `CurrentSlotID`
- 无需重建索引

---

## 🔄 **更新历史**

### 最新架构（当前）

- ✅ 物品位置自存储
- ✅ 单一数据源（AllItems）
- ✅ GUID 唯一性
- ✅ 容器嵌套支持
- ✅ 游离物品支持
- ✅ 数据一致性验证

### 主要改进

1. **移除了 `ItemLocationIndex`**：不再需要单独的位置索引
2. **移除了 `Container->Items`**：容器不再存储物品数据副本
3. **简化了移动逻辑**：从 O(n) 降低到 O(1)
4. **增强了数据一致性**：单一数据源，不会出现不一致

---

## 🤝 **贡献指南**

### 添加新功能

1. 在 `GaiaInventorySubsystem.h` 中声明函数
2. 在 `GaiaInventorySubsystem.cpp` 中实现
3. 在 `GaiaInventoryTestHelper` 中添加测试
4. 更新相关文档

### 报告问题

1. 运行 `Gaia.Inventory.PrintAllContainers` 和 `PrintAllItems`
2. 保存日志输出
3. 描述预期行为和实际行为
4. 提供复现步骤

---

## 📞 **获取帮助**

- 查看 **[系统设计说明.md](系统设计说明.md)** 了解架构
- 查看 **[测试指南.md](测试指南.md)** 学习如何测试
- 查看 **[控制台命令使用指南.md](控制台命令使用指南.md)** 学习如何调试
- 使用控制台命令检查系统状态
- 运行测试验证功能

---

**Gaia 库存系统 - 简洁、高效、可扩展** ✨

