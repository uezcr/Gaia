# 网络测试Actor使用指南

## 📋 **概述**

`AGaiaInventoryNetworkTestActor` 是专门用于测试库存系统联机功能的测试Actor，可以验证：
- RPC组件初始化
- 容器独占访问
- 物品移动同步
- 服务器广播
- 容器所有者注册

---

## 🚀 **快速开始**

### 1. 创建测试关卡

1. 在UE编辑器中创建一个新关卡（或使用现有的）
2. 在关卡中放置 `GaiaInventoryNetworkTestActor`
   - 在 Place Actors 面板搜索 "GaiaInventoryNetworkTestActor"
   - 或使用蓝图创建

### 2. 配置PIE设置

**编辑 → 编辑器偏好设置 → Play**

```
Multiplayer Options:
  ✅ Run Under One Process: 勾选
  ✅ Play NumberOf Clients: 2
  ✅ Play Mode: Play As Listen Server

Network Settings:
  ✅ Use Single Process: 勾选
```

### 3. 运行测试

1. 点击编辑器的 **Play** 按钮（或按 Alt+P）
2. 等待2秒（默认延迟）
3. 观察 Output Log 窗口的测试结果

---

## 🎮 **测试Actor属性**

### Test Control（测试控制）

| 属性 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `bAutoRunTests` | bool | true | 是否在BeginPlay时自动运行测试 |
| `TestStartDelay` | float | 2.0 | 延迟多少秒后开始测试（等待客户端连接） |
| `bVerboseLogging` | bool | true | 是否启用详细日志 |

### Test Options（测试选项）

| 属性 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `bTestRPCComponent` | bool | true | 测试RPC组件初始化 |
| `bTestExclusiveAccess` | bool | true | 测试容器独占访问 |
| `bTestItemMovement` | bool | true | 测试物品移动同步 |
| `bTestBroadcast` | bool | true | 测试服务器广播 |
| `bTestContainerOwnership` | bool | true | 测试容器所有者注册 |

---

## 📝 **测试用例详解**

### 测试1: RPC组件初始化

**目的**：验证所有玩家控制器都有RPC组件

**步骤**：
1. 获取所有玩家控制器
2. 检查每个玩家是否有 `UGaiaInventoryRPCComponent`
3. 显示拥有的容器和打开的世界容器数量

**预期结果**：
```
✅ 所有玩家控制器都有RPC组件
```

**失败原因**：
- PlayerController没有添加RPC组件
- 组件未正确初始化

---

### 测试2: 容器独占访问

**目的**：验证一个容器同时只能被一个玩家打开

**步骤**：
1. 创建测试箱子
2. 玩家1打开箱子 → 应该成功
3. 玩家2尝试打开同一个箱子 → 应该被拒绝
4. 检查占用状态
5. 玩家1关闭箱子
6. 玩家2重新打开箱子 → 应该成功

**预期结果**：
```
✅ 玩家1成功打开箱子
✅ 玩家2被正确拒绝: 容器正被 Player1 使用
✅ 容器占用状态正确
✅ 玩家1关闭后，玩家2成功打开箱子
```

**失败原因**：
- 独占访问逻辑错误
- 容器状态未正确清理

---

### 测试3: 物品移动同步

**目的**：验证物品移动操作正确执行和数据一致性

**步骤**：
1. 创建2个测试容器，注册为玩家1拥有
2. 创建测试物品（数量10）并添加到容器1
3. 移动5个物品到容器2
4. 验证数据一致性

**预期结果**：
```
✅ 成功移动 5 个物品
✅ 源物品数量正确: 5
```

**失败原因**：
- 物品移动逻辑错误
- 数据不一致

---

### 测试4: 服务器广播

**目的**：验证容器更新时的广播机制

**步骤**：
1. 创建测试容器
2. 注册所有者为玩家1
3. 验证所有者正确
4. 触发广播

**预期结果**：
```
✅ 容器所有者正确
✅ 广播已触发
```

**失败原因**：
- 所有者注册失败
- 广播逻辑错误

---

### 测试5: 容器所有者注册

**目的**：验证容器所有权的注册和注销

**步骤**：
1. 创建测试背包
2. 注册所有者为玩家1
3. 验证所有者
4. 注销所有者
5. 验证注销结果

**预期结果**：
```
✅ 所有者注册成功
✅ 所有者注销成功
```

**失败原因**：
- 注册逻辑错误
- 数据结构问题

---

## 📊 **测试输出示例**

### 成功输出

```
LogGaia: Warning: ========================================
LogGaia: Warning: 网络测试将在 2.0 秒后开始...
LogGaia: Warning: ========================================

LogGaia: Warning: ========================================
LogGaia: Warning: 开始运行库存系统网络测试
LogGaia: Warning: ========================================

LogGaia: Warning: ========================================
LogGaia: Warning: 测试1: RPC组件初始化
LogGaia: Warning: ========================================
LogGaia: Log: 找到 2 个玩家控制器
LogGaia: Log:   玩家0 (BP_PlayerController_C_0): ✅ 有RPC组件
LogGaia: Log:   玩家1 (BP_PlayerController_C_1): ✅ 有RPC组件
LogGaia: Display: [✅] RPC组件初始化 通过 - 2/2 玩家有RPC组件

LogGaia: Warning: ========================================
LogGaia: Warning: 测试2: 容器所有者注册
LogGaia: Warning: ========================================
LogGaia: Log: 创建测试背包: 7C148DE844C8C335B8EA198A4D6E6E5C
LogGaia: Log:   ✅ 所有者注册成功
LogGaia: Log:   ✅ 所有者注销成功
LogGaia: Display: [✅] 容器所有者注册 通过 - 注册和注销机制正常工作

...

LogGaia: Warning: ========================================
LogGaia: Display: ✅ 所有网络测试通过！
LogGaia: Warning: ========================================
```

### 失败输出

```
LogGaia: Error:   玩家1 (BP_PlayerController_C_1): ❌ 没有RPC组件
LogGaia: Error: [❌] RPC组件初始化 失败 - 1/2 玩家有RPC组件
LogGaia: Error: ❌ 部分网络测试失败！
```

---

## 🎯 **手动测试**

如果不想自动运行，可以手动触发：

### 蓝图方式

1. 在关卡中放置 `GaiaInventoryNetworkTestActor`
2. 设置 `Auto Run Tests` = false
3. 创建蓝图逻辑：
   ```
   Get Actor of Class (GaiaInventoryNetworkTestActor)
     ↓
   Run All Tests
   ```

### C++方式

```cpp
void AYourGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // 延迟5秒后手动运行测试
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        TArray<AActor*> TestActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGaiaInventoryNetworkTestActor::StaticClass(), TestActors);
        
        for (AActor* Actor : TestActors)
        {
            if (AGaiaInventoryNetworkTestActor* TestActor = Cast<AGaiaInventoryNetworkTestActor>(Actor))
            {
                TestActor->RunAllTests();
            }
        }
    }, 5.0f, false);
}
```

---

## 🔧 **调试技巧**

### 1. 查看详细日志

设置 `bVerboseLogging = true`，可以看到更多信息：
```
LogGaia: Verbose:     - 拥有的容器: 2
LogGaia: Verbose:     - 打开的世界容器: 1
LogGaia: Verbose: [网络] 注册容器所有者: 玩家=BP_PlayerController_C_0, 容器=...
```

### 2. 单独运行特定测试

取消勾选不需要的测试：
```
bTestRPCComponent = true
bTestExclusiveAccess = false  // 跳过此测试
bTestItemMovement = false
bTestBroadcast = false
bTestContainerOwnership = true  // 只运行这个测试
```

### 3. 调整延迟时间

如果客户端连接较慢，增加延迟：
```
TestStartDelay = 5.0  // 等待5秒
```

### 4. 过滤日志

在 Output Log 窗口中，输入 `LogGaia` 只显示库存系统日志。

---

## ⚠️ **注意事项**

### 1. 需要至少2个玩家

某些测试（如容器独占访问）需要至少2个玩家。确保PIE设置为2个或更多玩家。

### 2. 需要RPC组件

测试假设玩家控制器已经添加了 `UGaiaInventoryRPCComponent`。如果没有，测试会失败。

解决方案：
```cpp
// 在你的 PlayerController::BeginPlay() 中
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        UGaiaInventoryRPCComponent* RPCComp = NewObject<UGaiaInventoryRPCComponent>(this);
        RPCComp->RegisterComponent();
    }
}
```

### 3. 只在服务器端运行

测试Actor只在服务器端执行测试，客户端不会运行。

### 4. 测试顺序

测试按照固定顺序执行：
1. RPC组件初始化
2. 容器所有者注册
3. 容器独占访问
4. 物品移动同步
5. 服务器广播

某些测试可能依赖于之前的测试结果。

---

## 🚀 **扩展测试**

可以轻松添加新的测试用例：

```cpp
// GaiaInventoryNetworkTestActor.h
bool Test_MyCustomTest();

// GaiaInventoryNetworkTestActor.cpp
bool AGaiaInventoryNetworkTestActor::Test_MyCustomTest()
{
    LogSeparator(TEXT("测试X: 我的自定义测试"));
    
    // 测试逻辑...
    
    LogTestResult(TEXT("我的自定义测试"), bSuccess, TEXT("详细信息"));
    return bSuccess;
}

// 在 RunAllTests() 中调用
if (bTestMyCustom)
{
    if (!Test_MyCustomTest())
    {
        bAllTestsPassed = false;
    }
}
```

---

## 📚 **相关文档**

- [网络测试计划.md](网络测试计划.md) - 完整的测试计划和用例
- [容器独占访问功能说明.md](容器独占访问功能说明.md) - 独占访问机制
- [服务器广播功能说明.md](服务器广播功能说明.md) - 广播系统

---

**准备好测试你的联机库存系统了吗？** 🎮✨

