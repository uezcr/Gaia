# DisplayName 功能说明

## ✅ **已添加的功能**

### 1. **数据结构扩展**

#### **FGaiaItemInstance** 添加字段：
```cpp
/** 调试显示名称（用于日志输出） */
UPROPERTY(BlueprintReadWrite, Category = "Debug")
FString DebugDisplayName;

/** 获取简短的调试名称 */
FString GetDebugName() const;  // 返回 "木材(x10)" 或 "TestItem(x10)"

/** 获取简短的UID（前8位） */
FString GetShortUID() const;  // 返回 "2C4EAAC7" 而不是完整GUID
```

#### **FGaiaContainerInstance** 添加字段：
```cpp
/** 调试显示名称（用于日志输出） */
UPROPERTY(BlueprintReadWrite, Category = "Debug")
FString DebugDisplayName;

/** 获取简短的调试名称 */
FString GetDebugName() const;  // 返回 "背包1" 或 "TestContainer"

/** 获取简短的UID（前8位） */
FString GetShortUID() const;  // 返回 "696F35CD" 而不是完整GUID
```

---

### 2. **调试辅助函数**

#### **UGaiaInventorySubsystem** 添加：
```cpp
/** 设置物品的调试显示名称 */
void SetItemDebugName(const FGuid& ItemUID, const FString& DebugName);

/** 设置容器的调试显示名称 */
void SetContainerDebugName(const FGuid& ContainerUID, const FString& DebugName);
```

---

## 📖 **使用方法**

### 在测试代码中设置DisplayName

```cpp
// 创建容器后立即设置名称
FGuid Container1UID = InventorySystem->CreateContainer(TEXT("TestContainer"));
InventorySystem->SetContainerDebugName(Container1UID, TEXT("背包1"));

FGuid Container2UID = InventorySystem->CreateContainer(TEXT("TestContainer"));
InventorySystem->SetContainerDebugName(Container2UID, TEXT("背包2"));

// 创建物品后设置名称
FGaiaItemInstance Item = InventorySystem->CreateItemInstance(TEXT("TestItem"), 10);
InventorySystem->SetItemDebugName(Item.InstanceUID, TEXT("木材"));
```

---

## 📊 **日志对比**

### 修改前的日志（难以阅读）：
```
LogGaia: Warning: 【移除物品】开始: ItemUID=2C4EAAC74B4FA0E8FC0206B0DA56CAC6, 
                   容器=C532F1A747B7FEB71E8F8293EC14F18B, 槽位=2
LogGaia: Warning: 【移除物品】清空槽位引用: 容器=C532F1A747B7FEB71E8F8293EC14F18B, 
                   槽位ID=2, 槽位索引=2, 原引用=2C4EAAC74B4FA0E8FC0206B0DA56CAC6
```

### 修改后的日志（清晰易读）：
```
LogGaia: Warning: 【移除物品】开始: 物品=木材(x10) [2C4EAAC7], 容器=背包2 [C532F1A7], 槽位=2
LogGaia: Warning: 【移除物品】清空槽位引用: 容器=背包2 [C532F1A7], 槽位ID=2, 槽位索引=2
```

---

## 🎯 **主要优势**

### 1. **日志更易读**
- 使用中文名称（如"木材"、"背包1"）而不是DefinitionID
- 显示数量（如"x10"）
- 短UID（8位）代替完整GUID（32位）

### 2. **灵活性**
- 可以在测试中设置任意名称
- 如果未设置DisplayName，自动使用DefinitionID
- 不影响游戏逻辑，纯调试用途

### 3. **易于追踪**
- 通过短UID快速关联日志
- 通过名称理解物品/容器含义
- 减少日志行宽，更容易阅读

---

## 🔧 **建议的日志更新模式**

### 关键日志模式：
```cpp
// 物品相关日志
UE_LOG(LogGaia, Warning, TEXT("【操作】描述: 物品=%s [%s], ..."),
    *Item->GetDebugName(),      // "木材(x10)"
    *Item->GetShortUID());      // "2C4EAAC7"

// 容器相关日志
UE_LOG(LogGaia, Warning, TEXT("【操作】描述: 容器=%s [%s], ..."),
    *Container->GetDebugName(),  // "背包1"
    *Container->GetShortUID());  // "696F35CD"

// 移动操作日志（涉及双方）
UE_LOG(LogGaia, Warning, TEXT("【移动】物品=%s [%s] 从 容器=%s [%s] 槽位%d -> 容器=%s [%s] 槽位%d"),
    *Item->GetDebugName(), *Item->GetShortUID(),
    *SourceContainer->GetDebugName(), *SourceContainer->GetShortUID(), SourceSlot,
    *TargetContainer->GetDebugName(), *TargetContainer->GetShortUID(), TargetSlot);
```

---

## ✅ **已更新的日志**

1. **RemoveItemFromContainer** - 使用了简短名称
2. **测试代码** - 设置了容器DisplayName（背包1、背包2）

---

## 📝 **待更新的日志（建议）**

由于日志较多，可以渐进式更新：

### 优先级1：高频调试日志
- ✅ `RemoveItemFromContainer`
- [ ] `DestroyItem`
- [ ] `MoveToEmptySlot`
- [ ] `TryStackItems`

### 优先级2：移动相关
- [ ] `MoveItem`
- [ ] `SwapItems`
- [ ] `MoveItemWithinContainer`

### 优先级3：其他操作
- [ ] `AddItemToContainer`
- [ ] `CreateItemInstance`
- [ ] `CreateContainer`

---

## 💡 **最佳实践**

### 1. **测试代码中总是设置DisplayName**
```cpp
// 好的做法
FGuid Container = CreateContainer(...);
SetContainerDebugName(Container, TEXT("玩家背包"));

// 不好的做法（日志会显示DefinitionID）
FGuid Container = CreateContainer(...);
// 没有设置DisplayName
```

### 2. **使用中文名称**
```cpp
// 好 - 易读
SetItemDebugName(Item, TEXT("木材"));
SetContainerDebugName(Container, TEXT("背包1"));

// 可以，但不够直观
SetItemDebugName(Item, TEXT("Wood"));
SetContainerDebugName(Container, TEXT("Container1"));
```

### 3. **短UID用于关联日志**
```cpp
// 可以通过短UID快速搜索相关日志
// 日志：物品=木材(x10) [2C4EAAC7]
// 搜索：2C4EAAC7
// 找到所有关于这个物品的操作
```

---

## 🚀 **当前状态**

- ✅ 数据结构已扩展
- ✅ 辅助函数已实现
- ✅ 测试代码已更新（设置DisplayName）
- ✅ 部分日志已更新（RemoveItemFromContainer）
- ⏳ 其他日志可以渐进式更新

---

## 📖 **示例效果**

### 完整的移动操作日志：
```
LogGaia: Log: === 开始移动物品测试 ===
LogGaia: Log: 创建容器: 背包1 [696F35CD]
LogGaia: Log: 创建容器: 背包2 [C532F1A7]
LogGaia: Log: 创建物品: 木材(x10) [8A3B2C1D]
LogGaia: Log: 添加物品 木材(x10) 到 背包1
LogGaia: Log: 移动 木材(x5) [8A3B2C1D] 从 背包1 槽位0 -> 背包2 槽位0
LogGaia: Warning: 【堆叠】源物品数量为0，准备删除: 木材(x0) [2C4EAAC7]
LogGaia: Warning: 【移除物品】开始: 物品=木材(x0) [2C4EAAC7], 容器=背包2 [C532F1A7], 槽位=2
LogGaia: Warning: 【移除物品】清空槽位引用: 容器=背包2 [C532F1A7], 槽位ID=2
LogGaia: Warning: 【删除物品】完成: 木材 [2C4EAAC7] 已从AllItems中移除
LogGaia: Log: 库存数据一致性验证通过！✓
```

清晰、易读、易追踪！

---

**DisplayName功能已就绪！现在日志更容易理解了！** 🎉

