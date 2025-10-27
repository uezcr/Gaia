# GaiaInventoryRPCComponent 使用指南

## 📋 **概述**

`UGaiaInventoryRPCComponent` 是库存系统的网络同步组件，负责在客户端和服务器之间传递库存操作请求和同步数据。

### 🎯 **设计理念**

```
┌─────────────────────────────────────┐
│  服务器 (Authority)                 │
│  ├─ UGaiaInventorySubsystem         │ ← 唯一数据源
│  │   - AllItems (权威数据)         │
│  │   - Containers (权威数据)       │
│  │                                  │
│  └─ UGaiaInventoryRPCComponent      │ ← RPC通道
│      - 处理客户端请求               │
│      - 发送数据给客户端             │
└─────────────────────────────────────┘
         ↓ RPC 通信
┌─────────────────────────────────────┐
│  客户端                             │
│  └─ UGaiaInventoryRPCComponent      │ ← RPC通道 + 本地缓存
│      - 发送操作请求                 │
│      - 接收并缓存数据               │
│      - 触发UI更新事件               │
└─────────────────────────────────────┘
```

---

## 🚀 **快速开始**

### 1. 添加组件到 PlayerController

#### C++ 方式

```cpp
// YourPlayerController.h
UCLASS()
class AYourPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UGaiaInventoryRPCComponent* InventoryRPCComponent;
};

// YourPlayerController.cpp
void AYourPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 创建并注册RPC组件
    InventoryRPCComponent = NewObject<UGaiaInventoryRPCComponent>(this, TEXT("InventoryRPC"));
    InventoryRPCComponent->RegisterComponent();

    // 服务器端：初始化玩家背包
    if (HasAuthority())
    {
        if (UGaiaInventorySubsystem* InventorySystem = GetWorld()->GetSubsystem<UGaiaInventorySubsystem>())
        {
            // 创建玩家背包容器
            FGuid BackpackUID = InventorySystem->CreateContainer(TEXT("PlayerBackpack"));
            
            // 设置为该玩家拥有的容器
            InventoryRPCComponent->OwnedContainerUIDs.Add(BackpackUID);
            
            // TODO: 也可以创建玩家装备槽等其他容器
            
            // 首次同步数据给客户端
            InventoryRPCComponent->RequestRefreshInventory();
        }
    }
}
```

#### 蓝图方式

1. 打开你的 PlayerController 蓝图
2. 在组件面板点击 "Add Component"
3. 搜索 "Gaia Inventory RPC Component"
4. 添加组件
5. 在 BeginPlay 中初始化（服务器端）

```
BeginPlay
  ↓
Has Authority?
  ↓ 是
Get Inventory Subsystem
  ↓
Create Container ("PlayerBackpack")
  ↓
Add to Owned Container UIDs
  ↓
Request Refresh Inventory
```

---

## 🎮 **客户端使用**

### 在 UI 中调用操作

#### 移动物品

```cpp
// InventoryWidget.cpp

void UInventoryWidget::OnItemDragged(
    const FGuid& ItemUID,
    const FGuid& TargetContainer,
    int32 TargetSlot,
    int32 Quantity)
{
    // 获取 PlayerController
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
        return;

    // 获取 RPC 组件
    UGaiaInventoryRPCComponent* RPCComp = PC->FindComponentByClass<UGaiaInventoryRPCComponent>();
    if (!RPCComp)
        return;

    // 请求移动物品
    RPCComp->RequestMoveItem(ItemUID, TargetContainer, TargetSlot, Quantity);
}
```

#### 蓝图方式

```
Get Owning Player
  ↓
Get Component By Class (Gaia Inventory RPC Component)
  ↓
Request Move Item
  - Item UID: [拖拽的物品]
  - Target Container UID: [目标容器]
  - Target Slot ID: [目标槽位]
  - Quantity: [移动数量]
```

---

### 读取本地缓存数据

```cpp
// InventoryWidget.cpp

void UInventoryWidget::UpdateItemDisplay(const FGuid& ItemUID)
{
    UGaiaInventoryRPCComponent* RPCComp = GetOwningPlayer()->FindComponentByClass<UGaiaInventoryRPCComponent>();
    if (!RPCComp)
        return;

    // 从本地缓存获取物品数据
    FGaiaItemInstance Item;
    if (RPCComp->GetCachedItem(ItemUID, Item))
    {
        // 更新UI显示
        ItemNameText->SetText(FText::FromString(Item.DebugDisplayName));
        ItemQuantityText->SetText(FText::AsNumber(Item.Quantity));
    }
}

void UInventoryWidget::DisplayContainer(const FGuid& ContainerUID)
{
    UGaiaInventoryRPCComponent* RPCComp = GetOwningPlayer()->FindComponentByClass<UGaiaInventoryRPCComponent>();
    if (!RPCComp)
        return;

    // 从本地缓存获取容器数据
    FGaiaContainerInstance Container;
    if (RPCComp->GetCachedContainer(ContainerUID, Container))
    {
        // 遍历槽位
        for (const FGaiaSlotInfo& Slot : Container.Slots)
        {
            if (!Slot.IsEmpty())
            {
                // 获取槽位中的物品
                FGaiaItemInstance Item;
                if (RPCComp->GetCachedItem(Slot.ItemInstanceUID, Item))
                {
                    // 显示物品
                    CreateSlotWidget(Slot.SlotID, Item);
                }
            }
        }
    }
}
```

---

### 监听更新事件

```cpp
// InventoryWidget.h
UCLASS()
class UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void OnOperationFailed(int32 ErrorCode, const FString& ErrorMessage);

private:
    UPROPERTY()
    UGaiaInventoryRPCComponent* CachedRPCComponent;
};

// InventoryWidget.cpp
void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 获取并缓存RPC组件
    CachedRPCComponent = GetOwningPlayer()->FindComponentByClass<UGaiaInventoryRPCComponent>();
    
    if (CachedRPCComponent)
    {
        // 绑定事件
        CachedRPCComponent->OnInventoryUpdated.AddDynamic(this, &UInventoryWidget::OnInventoryUpdated);
        CachedRPCComponent->OnOperationFailed.AddDynamic(this, &UInventoryWidget::OnOperationFailed);
    }
}

void UInventoryWidget::NativeDestruct()
{
    // 解绑事件
    if (CachedRPCComponent)
    {
        CachedRPCComponent->OnInventoryUpdated.RemoveDynamic(this, &UInventoryWidget::OnInventoryUpdated);
        CachedRPCComponent->OnOperationFailed.RemoveDynamic(this, &UInventoryWidget::OnOperationFailed);
    }

    Super::NativeDestruct();
}

void UInventoryWidget::OnInventoryUpdated()
{
    // 库存数据已更新，刷新UI
    RefreshAllSlots();
}

void UInventoryWidget::OnOperationFailed(int32 ErrorCode, const FString& ErrorMessage)
{
    // 显示错误提示
    ShowErrorMessage(ErrorMessage);
}
```

#### 蓝图方式

1. 在 Widget 的 Event Construct 中：
```
Get Owning Player
  ↓
Get Component By Class (Gaia Inventory RPC Component)
  ↓
Bind Event to On Inventory Updated (自定义事件: RefreshUI)
  ↓
Bind Event to On Operation Failed (自定义事件: ShowError)
```

2. 创建自定义事件 "RefreshUI"：
```
RefreshUI
  ↓
Get Owned Container UIDs
  ↓
For Each Container
  ↓
Get Cached Container
  ↓
Update UI
```

---

## 🌐 **世界容器（箱子）使用**

### 打开箱子

```cpp
// 玩家与箱子交互
void AChestActor::OnPlayerInteract(APlayerController* Player)
{
    if (!Player)
        return;

    UGaiaInventoryRPCComponent* RPCComp = Player->FindComponentByClass<UGaiaInventoryRPCComponent>();
    if (!RPCComp)
        return;

    // 请求打开世界容器
    RPCComp->RequestOpenWorldContainer(ChestContainerUID);
}
```

### 关闭箱子

```cpp
void UInventoryWidget::OnCloseChest(const FGuid& ChestContainerUID)
{
    UGaiaInventoryRPCComponent* RPCComp = GetOwningPlayer()->FindComponentByClass<UGaiaInventoryRPCComponent>();
    if (!RPCComp)
        return;

    // 请求关闭世界容器
    RPCComp->RequestCloseWorldContainer(ChestContainerUID);
}
```

### 监听容器打开/关闭事件

```cpp
// 绑定事件
RPCComponent->OnContainerOpened.AddDynamic(this, &UInventoryWidget::OnContainerOpened);
RPCComponent->OnContainerClosed.AddDynamic(this, &UInventoryWidget::OnContainerClosed);

void UInventoryWidget::OnContainerOpened(const FGuid& ContainerUID)
{
    // 显示容器UI
    ShowContainerPanel(ContainerUID);
}

void UInventoryWidget::OnContainerClosed(const FGuid& ContainerUID)
{
    // 隐藏容器UI
    HideContainerPanel(ContainerUID);
}
```

---

## 🔧 **服务器端扩展**

### 自定义权限检查

目前权限检查是TODO状态，你可以在 `GaiaInventorySubsystem` 中实现：

```cpp
// GaiaInventorySubsystem.h
public:
    /** 检查玩家是否有权限访问物品 */
    bool CanPlayerAccessItem(APlayerController* Player, const FGuid& ItemUID) const;

    /** 检查玩家是否有权限访问容器 */
    bool CanPlayerAccessContainer(APlayerController* Player, const FGuid& ContainerUID) const;

    /** 检查玩家是否在容器范围内 */
    bool IsPlayerInRangeOfContainer(APlayerController* Player, const FGuid& ContainerUID) const;

private:
    /** 玩家拥有的容器映射 */
    TMap<APlayerController*, TArray<FGuid>> PlayerOwnedContainers;
```

然后在 `ServerMoveItem_Implementation` 中调用：

```cpp
void UGaiaInventoryRPCComponent::ServerMoveItem_Implementation(...)
{
    UGaiaInventorySubsystem* InventorySystem = GetInventorySubsystem();
    APlayerController* PC = GetOwningPlayerController();

    // 权限检查
    if (!InventorySystem->CanPlayerAccessItem(PC, ItemUID))
    {
        ClientOperationFailed(2, TEXT("无权访问该物品"));
        return;
    }

    if (!InventorySystem->CanPlayerAccessContainer(PC, TargetContainerUID))
    {
        ClientOperationFailed(3, TEXT("无权访问该容器"));
        return;
    }

    // 执行移动...
}
```

---

## 📊 **网络流量优化**

### 当前实现

- ✅ **按需同步**：只同步玩家拥有的容器和打开的世界容器
- ✅ **客户端缓存**：避免重复请求
- ✅ **RPC验证**：防止恶意数据

### 未来优化（可选）

#### 1. 增量同步

使用 `ClientReceiveInventoryDelta` 而不是 `ClientReceiveInventoryData`：

```cpp
void UGaiaInventorySubsystem::BroadcastInventoryChange(
    const TArray<FGuid>& ChangedItemUIDs,
    const TArray<FGuid>& RemovedItemUIDs)
{
    // 只发送变化的数据
    TArray<FGaiaItemInstance> UpdatedItems;
    for (const FGuid& ItemUID : ChangedItemUIDs)
    {
        FGaiaItemInstance Item;
        if (FindItemByUID(ItemUID, Item))
        {
            UpdatedItems.Add(Item);
        }
    }

    // 发送增量更新
    RPCComponent->ClientReceiveInventoryDelta(UpdatedItems, RemovedItemUIDs, {});
}
```

#### 2. 数据压缩

对于大量物品，可以自定义序列化：

```cpp
// 自定义网络序列化
bool FGaiaItemInstance::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
    // 只序列化必要字段，减少网络流量
    Ar << InstanceUID;
    Ar << ItemDefinitionID;
    Ar << Quantity;
    // ...
    
    bOutSuccess = true;
    return true;
}
```

---

## 🐛 **调试技巧**

### 1. 查看网络日志

```cpp
// 在组件中已经有详细的日志输出
UE_LOG(LogGaia, Log, TEXT("[网络] 客户端收到库存数据: %d 个物品"), Items.Num());
```

在控制台输入：
```
Log LogGaia Verbose
```

### 2. PIE 多窗口测试

1. 编辑器设置 → PIE
2. Play Mode → Play As Listen Server
3. Number of Players → 2
4. 点击 Play

可以测试一个服务器 + 一个客户端

### 3. 检查复制状态

```cpp
// 在组件中添加调试函数
UFUNCTION(BlueprintCallable, Category = "Debug")
void DebugPrintNetworkState()
{
    UE_LOG(LogGaia, Warning, TEXT("=== 网络状态调试 ==="));
    UE_LOG(LogGaia, Warning, TEXT("Owner Role: %d"), GetOwnerRole());
    UE_LOG(LogGaia, Warning, TEXT("拥有的容器: %d"), OwnedContainerUIDs.Num());
    UE_LOG(LogGaia, Warning, TEXT("缓存的物品: %d"), CachedItems.Num());
    UE_LOG(LogGaia, Warning, TEXT("缓存的容器: %d"), CachedContainers.Num());
}
```

---

## ⚠️ **注意事项**

### 1. 客户端缓存是只读的

```cpp
// ❌ 错误：直接修改客户端缓存
FGaiaItemInstance* Item = CachedItems.Find(ItemUID);
Item->Quantity = 10; // 无效！不会同步到服务器

// ✅ 正确：通过RPC请求修改
RPCComponent->RequestMoveItem(...);
```

### 2. 服务器权威

所有逻辑都在服务器执行，客户端只负责：
- 发送请求
- 接收数据
- 更新UI

### 3. 网络延迟

客户端操作到UI更新有延迟（RPC往返时间）：

```
客户端操作 → RPC → 服务器处理 → RPC → 客户端更新UI
    0ms      20ms      1ms       20ms      41ms
```

如果需要即时反馈，可以实现**客户端预测**（高级）。

---

## 📝 **完整示例**

### 创建一个完整的背包UI

```cpp
// BackpackWidget.h
UCLASS()
class UBackpackWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Backpack")
    void Initialize(const FGuid& InBackpackContainerUID);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void RefreshDisplay();

private:
    UPROPERTY()
    UGaiaInventoryRPCComponent* RPCComponent;

    FGuid BackpackContainerUID;
};

// BackpackWidget.cpp
void UBackpackWidget::Initialize(const FGuid& InBackpackContainerUID)
{
    BackpackContainerUID = InBackpackContainerUID;
    RefreshDisplay();
}

void UBackpackWidget::NativeConstruct()
{
    Super::NativeConstruct();

    RPCComponent = GetOwningPlayer()->FindComponentByClass<UGaiaInventoryRPCComponent>();
    if (RPCComponent)
    {
        RPCComponent->OnInventoryUpdated.AddDynamic(this, &UBackpackWidget::OnInventoryUpdated);
    }
}

void UBackpackWidget::NativeDestruct()
{
    if (RPCComponent)
    {
        RPCComponent->OnInventoryUpdated.RemoveDynamic(this, &UBackpackWidget::OnInventoryUpdated);
    }

    Super::NativeDestruct();
}

void UBackpackWidget::OnInventoryUpdated()
{
    RefreshDisplay();
}

void UBackpackWidget::RefreshDisplay()
{
    if (!RPCComponent)
        return;

    // 获取背包容器
    FGaiaContainerInstance Container;
    if (!RPCComponent->GetCachedContainer(BackpackContainerUID, Container))
        return;

    // 清空当前显示
    ClearSlots();

    // 遍历槽位
    for (const FGaiaSlotInfo& Slot : Container.Slots)
    {
        if (!Slot.IsEmpty())
        {
            // 获取物品数据
            FGaiaItemInstance Item;
            if (RPCComponent->GetCachedItem(Slot.ItemInstanceUID, Item))
            {
                // 创建并显示物品图标
                CreateSlotWidget(Slot.SlotID, Item);
            }
        }
    }
}
```

---

## 🎯 **总结**

### ✅ **RPC Component 优势**

1. **轻量级** - 不存储数据，只负责通信
2. **解耦合** - UI 只需要访问 RPC Component
3. **易于使用** - 蓝图友好，事件驱动
4. **高效** - 按需同步，客户端缓存

### 📚 **相关文档**

- [基于WorldSubsystem的联机方案.md](基于WorldSubsystem的联机方案.md) - 架构设计详解
- [系统设计说明.md](系统设计说明.md) - 库存系统整体设计
- [执行流程说明.md](执行流程说明.md) - 操作流程说明

---

**准备好在你的游戏中使用联机库存系统了吗？** 🚀

