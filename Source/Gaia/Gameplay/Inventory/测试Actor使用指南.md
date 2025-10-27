# 库存系统测试Actor使用指南

## 📋 **概述**

`AGaiaInventoryTestActor` 是一个可以放置在关卡中的测试Actor，提供了灵活的测试控制选项，可以通过蓝图或代码运行各种库存系统测试。

---

## 🎮 **在编辑器中使用**

### 1. 放置测试Actor

1. 打开测试关卡（如 `TestLevel.umap`）
2. 在 **Place Actors** 面板中搜索 `GaiaInventoryTestActor`
3. 将其拖放到关卡中

### 2. 配置测试选项

选中放置的Actor，在 **Details** 面板中可以看到以下配置选项：

---

## ⚙️ **配置选项详解**

### **Test Control** 分类

#### `bAutoRunTests` (默认: false)
- **说明**: 是否在BeginPlay时自动运行测试
- **默认**: `false` - 需要手动在蓝图中启用
- **用途**: 开启后，游戏开始时自动执行测试

#### `bRunAllBasicTests` (默认: true)
- **说明**: 是否运行所有基础测试
- **依赖**: 需要 `bAutoRunTests = true`
- **用途**: 
  - `true` - 运行完整的测试套件（推荐用于验证整体功能）
  - `false` - 只运行选中的单独测试（推荐用于调试特定功能）

---

### **Individual Tests** 分类

> ⚠️ **注意**: 只有当 `bAutoRunTests = true` 且 `bRunAllBasicTests = false` 时，这些选项才会生效

#### `bTestContainerCreation` (默认: true)
- **说明**: 测试容器创建功能
- **测试内容**: 创建容器并验证UID有效性

#### `bTestItemCreation` (默认: true)
- **说明**: 测试物品创建功能
- **测试内容**: 创建物品实例并验证属性

#### `bTestAddAndFind` (默认: true)
- **说明**: 测试物品添加和查找功能
- **测试内容**: 
  - 添加物品到容器
  - 通过UID查找物品
  - 验证物品数量和位置

#### `bTestRemoveAndOrphan` (默认: true)
- **说明**: 测试物品移除和游离状态
- **测试内容**:
  - 从容器中移除物品
  - 验证物品进入游离状态
  - 测试完全删除物品

#### `bTestDataIntegrity` (默认: true)
- **说明**: 测试数据完整性验证
- **测试内容**:
  - 验证物品位置与槽位引用一致
  - 验证容器嵌套关系
  - 测试数据修复功能

#### `bTestContainerDeletion` (默认: true)
- **说明**: 测试容器删除功能
- **测试内容**:
  - 删除容器物品
  - 验证嵌套容器递归删除
  - 验证物品正确清理

#### `bTestMoveItems` (默认: true)
- **说明**: 测试物品移动功能
- **测试内容**:
  - 跨容器移动
  - 容器内移动
  - 堆叠移动
  - 交换移动
  - 拆分移动
  - 自动槽位分配

---

### **Performance Tests** 分类

#### `bRunPerformanceTest` (默认: false)
- **说明**: 是否运行性能测试
- **用途**: 测试大量物品和容器的性能表现

#### `PerformanceTestItemCount` (默认: 100)
- **说明**: 性能测试的物品数量
- **范围**: 10 - 10000
- **依赖**: 需要 `bRunPerformanceTest = true`

---

## 📖 **使用场景**

### 场景1: 完整测试（推荐用于CI/验收测试）

```
配置:
✅ bAutoRunTests = true
✅ bRunAllBasicTests = true
❌ bRunPerformanceTest = false
```

**效果**: 运行游戏后自动执行所有基础测试，验证系统完整功能

---

### 场景2: 调试特定功能（推荐用于开发调试）

```
配置:
✅ bAutoRunTests = true
❌ bRunAllBasicTests = false
✅ bTestMoveItems = true  （只测试移动功能）
❌ bTestContainerCreation = false
❌ bTestItemCreation = false
... (其他测试关闭)
```

**效果**: 只运行物品移动测试，快速定位问题

---

### 场景3: 性能测试

```
配置:
✅ bAutoRunTests = true
✅ bRunAllBasicTests = true
✅ bRunPerformanceTest = true
   PerformanceTestItemCount = 1000
```

**效果**: 运行所有基础测试 + 性能测试（1000个物品）

---

### 场景4: 手动触发（推荐用于蓝图控制）

```
配置:
❌ bAutoRunTests = false
```

**蓝图调用**:
- 在蓝图中获取TestActor引用
- 调用需要的测试函数（见下方函数列表）

---

## 🔧 **蓝图可调用函数**

所有函数都在 **Gaia|Test** 分类下：

### 1. `RunAllBasicTests()`
- **说明**: 运行所有基础测试
- **用途**: 验证整体功能

### 2. `RunSelectedTests()`
- **说明**: 根据Individual Tests配置运行选中的测试
- **用途**: 灵活控制测试范围

### 3. `RunContainerTest()`
- **说明**: 单独运行容器创建测试
- **用途**: 调试容器功能

### 4. `RunItemTest()`
- **说明**: 单独运行物品创建测试
- **用途**: 调试物品功能

### 5. `RunAddFindTest()`
- **说明**: 单独运行添加/查找测试
- **用途**: 调试添加和查询功能

### 6. `RunRemoveTest()`
- **说明**: 单独运行移除测试
- **用途**: 调试物品移除功能

### 7. `RunMoveTest()`
- **说明**: 单独运行移动测试
- **用途**: 调试物品移动功能

### 8. `RunDataIntegrityTest()`
- **说明**: 单独运行数据完整性测试
- **用途**: 验证数据一致性

### 9. `RunContainerDeletionTest()`
- **说明**: 单独运行容器删除测试
- **用途**: 调试容器删除功能

### 10. `RunPerformanceTest()`
- **说明**: 单独运行性能测试
- **用途**: 评估性能表现

---

## 🎨 **蓝图示例**

### 示例1: 通过按键触发测试

```
Event BeginPlay
  ↓
获取 TestActor 引用（通过Get Actor Of Class）
  ↓
保存为变量 TestActorRef
```

```
Input Action (按键 T)
  ↓
TestActorRef → RunAllBasicTests()
```

### 示例2: 延迟自动测试

```
Event BeginPlay
  ↓
Delay (2.0 秒)
  ↓
获取 TestActor 引用
  ↓
RunAllBasicTests()
```

### 示例3: UI按钮触发

```
Button_OnClicked (运行所有测试)
  ↓
TestActorRef → RunAllBasicTests()

Button_OnClicked (只测试移动)
  ↓
TestActorRef → RunMoveTest()
```

---

## 📊 **查看测试结果**

### 1. Output Log窗口

**位置**: Window → Developer Tools → Output Log

**过滤日志**:
```
在搜索框输入: LogGaia
```

**日志标记**:
- ✓ - 测试通过
- ✗ - 测试失败
- ⚠ - 警告信息

### 2. 日志文件

**位置**: `Saved/Logs/Gaia.log`

**搜索关键词**:
- `测试通过`
- `测试失败`
- `容器信息`
- `物品信息`

---

## 🐛 **常见问题**

### Q1: 配置了开关但测试没有运行？

**检查**:
1. ✅ `bAutoRunTests` 是否为 `true`
2. ✅ 游戏是否正常启动（按Play）
3. ✅ 查看Output Log是否有错误信息

---

### Q2: Individual Tests的开关不可编辑？

**原因**: 这些开关只有在 `bAutoRunTests = true` 且 `bRunAllBasicTests = false` 时才生效

**解决**:
1. 勾选 `bAutoRunTests`
2. 取消勾选 `bRunAllBasicTests`
3. 现在可以编辑Individual Tests了

---

### Q3: 如何只测试一个功能？

**步骤**:
1. 设置 `bAutoRunTests = true`
2. 设置 `bRunAllBasicTests = false`
3. 只勾选想要测试的功能（如 `bTestMoveItems`）
4. 其他测试开关设为 `false`
5. 运行游戏

---

### Q4: 性能测试需要多久？

**取决于**:
- `PerformanceTestItemCount` 设置的数量
- 机器性能

**参考时间**:
- 100个物品: ~1秒
- 1000个物品: ~5秒
- 10000个物品: ~30秒

---

## 📝 **最佳实践**

### 1. 开发阶段

```
推荐配置:
❌ bAutoRunTests = false (避免每次运行都测试)
```

- 在蓝图中添加按键绑定
- 需要测试时手动触发
- 节省启动时间

### 2. 提交代码前

```
推荐配置:
✅ bAutoRunTests = true
✅ bRunAllBasicTests = true
```

- 运行完整测试确保没有破坏功能
- 检查所有测试是否通过

### 3. 调试Bug

```
推荐配置:
✅ bAutoRunTests = true
❌ bRunAllBasicTests = false
✅ 只开启相关的单个测试
```

- 缩小测试范围
- 快速定位问题
- 减少日志干扰

### 4. 性能优化

```
推荐配置:
✅ bAutoRunTests = true
❌ bRunAllBasicTests = false
❌ 所有Individual Tests = false
✅ bRunPerformanceTest = true
   PerformanceTestItemCount = 1000+
```

- 专注性能测试
- 调整物品数量观察性能变化

---

## 🚀 **快速开始**

### 1分钟快速验证功能:

1. **放置Actor**: 在关卡中放置 `AGaiaInventoryTestActor`
2. **配置**: 勾选 `bAutoRunTests` 和 `bRunAllBasicTests`
3. **运行**: 按下 **Play** 按钮
4. **查看**: 打开 **Output Log**，搜索 `LogGaia`
5. **验证**: 查找 `所有测试通过！✓`

---

## 📚 **相关文档**

- **[测试指南.md](测试指南.md)** - 详细的测试说明
- **[移动功能测试计划.md](移动功能测试计划.md)** - 移动测试用例
- **[README.md](README.md)** - 系统文档索引

---

## ✅ **总结**

`AGaiaInventoryTestActor` 提供了灵活的测试控制：

- ✅ **自动测试**: BeginPlay自动运行
- ✅ **手动测试**: 蓝图/代码手动触发
- ✅ **选择性测试**: 可以只运行特定测试
- ✅ **性能测试**: 支持大量物品测试
- ✅ **易于配置**: 所有选项都暴露给蓝图

**推荐工作流**:
1. 开发时: 使用手动触发，避免自动测试
2. 调试时: 只开启相关测试，快速定位问题
3. 提交前: 运行完整测试，确保功能完整

---

**测试Actor已就绪！现在可以在蓝图中灵活控制测试了！** 🎉

