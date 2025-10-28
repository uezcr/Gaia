# UI系统诊断工具

> 用于诊断CommonUI系统为什么无法正常工作

---

## 🔍 诊断代码

将以下代码添加到你的PlayerController或测试Actor的BeginPlay中：

```cpp
void AYourActor::DiagnoseUISystem()
{
    UE_LOG(LogTemp, Warning, TEXT("========== UI系统诊断开始 =========="));
    
    // 1. 检查GameInstance
    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [1/7] GameInstance为null"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ [1/7] GameInstance存在: %s"), *GI->GetClass()->GetName());
    
    // 检查是否是CommonGameInstance
    if (UCommonGameInstance* CommonGI = Cast<UCommonGameInstance>(GI))
    {
        UE_LOG(LogTemp, Log, TEXT("✅ [1/7] 是CommonGameInstance子类"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [1/7] 不是CommonGameInstance！当前类型: %s"), *GI->GetClass()->GetName());
    }
    
    // 2. 检查LocalPlayer
    ULocalPlayer* LP = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LP)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [2/7] LocalPlayer为null"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ [2/7] LocalPlayer存在: %s"), *LP->GetClass()->GetName());
    
    // 检查是否是CommonLocalPlayer
    if (UCommonLocalPlayer* CommonLP = Cast<UCommonLocalPlayer>(LP))
    {
        UE_LOG(LogTemp, Log, TEXT("✅ [2/7] 是CommonLocalPlayer子类"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [2/7] 不是CommonLocalPlayer！当前类型: %s"), *LP->GetClass()->GetName());
    }
    
    // 3. 检查GameViewportClient
    UGameViewportClient* GVC = GetWorld()->GetGameViewport();
    if (!GVC)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [3/7] GameViewportClient为null"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("✅ [3/7] GameViewportClient存在: %s"), *GVC->GetClass()->GetName());
        
        if (GVC->GetClass()->GetName().Contains(TEXT("CommonGameViewportClient")))
        {
            UE_LOG(LogTemp, Log, TEXT("✅ [3/7] 是CommonGameViewportClient"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("⚠️ [3/7] 不是CommonGameViewportClient，当前: %s"), *GVC->GetClass()->GetName());
        }
    }
    
    // 4. 检查GameUIManagerSubsystem
    UGameUIManagerSubsystem* UIManager = GI->GetSubsystem<UGameUIManagerSubsystem>();
    if (!UIManager)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [4/7] GameUIManagerSubsystem为null - Subsystem没有被创建！"));
        UE_LOG(LogTemp, Error, TEXT("    可能原因: ShouldCreateSubsystem返回了false"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ [4/7] GameUIManagerSubsystem存在: %s"), *UIManager->GetClass()->GetName());
    
    // 5. 检查UIPolicy
    UGameUIPolicy* Policy = UIManager->GetCurrentUIPolicy();
    if (!Policy)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [5/7] UIPolicy为null"));
        UE_LOG(LogTemp, Error, TEXT("    检查Config/DefaultGame.ini中的DefaultUIPolicyClass配置"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ [5/7] UIPolicy存在: %s"), *Policy->GetClass()->GetName());
    
    // 6. 检查PrimaryGameLayout
    UPrimaryGameLayout* Layout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this);
    if (!Layout)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ [6/7] PrimaryGameLayout为null"));
        UE_LOG(LogTemp, Error, TEXT("    可能原因:"));
        UE_LOG(LogTemp, Error, TEXT("    - NotifyPlayerAdded没有被调用"));
        UE_LOG(LogTemp, Error, TEXT("    - BP_GaiaUIPolicy的LayoutClass未设置"));
        UE_LOG(LogTemp, Error, TEXT("    - LayoutClass设置错误"));
        
        // 尝试通过UIPolicy获取
        if (UCommonLocalPlayer* CommonLP = Cast<UCommonLocalPlayer>(LP))
        {
            UPrimaryGameLayout* PolicyLayout = Policy->GetRootLayout(CommonLP);
            if (PolicyLayout)
            {
                UE_LOG(LogTemp, Log, TEXT("✅ [6/7] 通过UIPolicy找到了Layout: %s"), *PolicyLayout->GetClass()->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("❌ [6/7] UIPolicy中也没有Layout - RootViewportLayouts是空的！"));
            }
        }
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ [6/7] PrimaryGameLayout存在: %s"), *Layout->GetClass()->GetName());
    
    // 7. 检查Layer Stacks
    // 这需要你的GaiaPrimaryGameLayout暴露getter，或者通过反射检查
    UE_LOG(LogTemp, Log, TEXT("✅ [7/7] UI系统基础检查完成"));
    
    UE_LOG(LogTemp, Warning, TEXT("========== UI系统诊断结束 =========="));
    UE_LOG(LogTemp, Warning, TEXT(""));
    UE_LOG(LogTemp, Warning, TEXT("🎯 总结:"));
    UE_LOG(LogTemp, Warning, TEXT("  - GameInstance: %s"), *GI->GetClass()->GetName());
    UE_LOG(LogTemp, Warning, TEXT("  - LocalPlayer: %s"), *LP->GetClass()->GetName());
    UE_LOG(LogTemp, Warning, TEXT("  - UIManager: %s"), UIManager ? *UIManager->GetClass()->GetName() : TEXT("null"));
    UE_LOG(LogTemp, Warning, TEXT("  - UIPolicy: %s"), Policy ? *Policy->GetClass()->GetName() : TEXT("null"));
    UE_LOG(LogTemp, Warning, TEXT("  - Layout: %s"), Layout ? *Layout->GetClass()->GetName() : TEXT("null"));
}
```

---

## 🔧 快速修复清单

根据诊断结果，按以下清单修复：

### ❌ 如果 GameUIManagerSubsystem 为 null

**问题：** Subsystem没有被创建

**解决方案1：** 覆盖 `ShouldCreateSubsystem`

在 `GaiaUIManagerSubsystem.h` 添加：

```cpp
virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
```

在 `GaiaUIManagerSubsystem.cpp` 添加：

```cpp
bool UGaiaUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // 不是专用服务器就创建
    return !CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance();
}
```

**解决方案2：** 修改配置section

修改 `Config/DefaultGame.ini`：

```ini
# 从这个：
[/Script/GaiaGame.GaiaUIManagerSubsystem]
DefaultUIPolicyClass=/Game/UI/BP_GaiaUIPolicy.BP_GaiaUIPolicy_C

# 改成这个：
[/Script/CommonGame.GameUIManagerSubsystem]
DefaultUIPolicyClass=/Game/UI/BP_GaiaUIPolicy.BP_GaiaUIPolicy_C
```

---

### ❌ 如果 UIPolicy 为 null

**问题：** UIPolicy没有被创建

**检查：**
1. `Config/DefaultGame.ini` 中的配置section是否正确
2. `DefaultUIPolicyClass` 路径是否正确
3. `BP_GaiaUIPolicy` 是否存在且编译成功

**修复：**

```ini
[/Script/CommonGame.GameUIManagerSubsystem]
DefaultUIPolicyClass=/Game/UI/BP_GaiaUIPolicy.BP_GaiaUIPolicy_C
```

---

### ❌ 如果 PrimaryGameLayout 为 null

**问题：** Layout没有被创建

**可能原因：**

1. **NotifyPlayerAdded没有被调用** → 检查是否使用CommonGameInstance
2. **LayoutClass未设置** → 打开BP_GaiaUIPolicy，设置Layout Class
3. **LayoutClass路径错误** → 确认WBP_GaiaPrimaryGameLayout存在

**修复步骤：**

1. 打开 `BP_GaiaUIPolicy`
2. Details面板 → **Layout Class**
3. 设置为：`WBP_GaiaPrimaryGameLayout`
4. 编译保存

---

## 📋 完整配置检查清单

### Config/DefaultEngine.ini

```ini
[/Script/EngineSettings.GameMapsSettings]
GameInstanceClass=/Script/GaiaGame.GaiaGameInstance

[/Script/Engine.Engine]
GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient
LocalPlayerClassName=/Script/GaiaGame.GaiaLocalPlayer
```

### Config/DefaultGame.ini

```ini
[/Script/CommonGame.GameUIManagerSubsystem]
DefaultUIPolicyClass=/Game/UI/BP_GaiaUIPolicy.BP_GaiaUIPolicy_C
```

**⚠️ 注意：** Section必须是 `CommonGame.GameUIManagerSubsystem`，不是 `GaiaGame.GaiaUIManagerSubsystem`！

### 蓝图配置

**BP_GaiaUIPolicy:**
- Parent Class: `GaiaUIPolicy`
- Layout Class: `WBP_GaiaPrimaryGameLayout`

**WBP_GaiaPrimaryGameLayout:**
- Parent Class: `GaiaPrimaryGameLayout`
- GameLayer: ✅ Is Variable
- ContainerLayer: ✅ Is Variable
- MenuLayer: ✅ Is Variable
- ModalLayer: ✅ Is Variable

---

## 🎯 预期日志输出

正确配置后，诊断日志应该显示：

```
✅ [1/7] GameInstance存在: GaiaGameInstance
✅ [1/7] 是CommonGameInstance子类
✅ [2/7] LocalPlayer存在: GaiaLocalPlayer
✅ [2/7] 是CommonLocalPlayer子类
✅ [3/7] GameViewportClient存在: CommonGameViewportClient
✅ [3/7] 是CommonGameViewportClient
✅ [4/7] GameUIManagerSubsystem存在: GaiaUIManagerSubsystem
✅ [5/7] UIPolicy存在: BP_GaiaUIPolicy_C
✅ [6/7] PrimaryGameLayout存在: WBP_GaiaPrimaryGameLayout_C
✅ [7/7] UI系统基础检查完成
```

---

**运行这个诊断工具，告诉我在哪一步失败了！**


