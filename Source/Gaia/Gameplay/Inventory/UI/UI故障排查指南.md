# UI系统故障排查指南

> **最后更新：** 2025-10-27

---

## 🔴 错误：无法获取PrimaryGameLayout

### 错误信息
```
LogGaia: Error: [Gaia UI] 无法获取PrimaryGameLayout
```

### 原因分析

这个错误说明 `PrimaryGameLayout` 没有被正确创建。可能的原因：

1. ❌ **UIPolicy未正确配置**
2. ❌ **Layout Widget类未设置**
3. ❌ **GameViewportClient不正确**
4. ❌ **LocalPlayer不是CommonLocalPlayer**

---

## ✅ 修复步骤（按顺序检查）

### 步骤1：重新编译C++代码

**刚刚修复了 `UGaiaUIPolicy::GetLayoutWidgetClass()`，需要重新编译！**

```bash
# 关闭UE编辑器
# 在VS或Rider中重新编译项目
# 重新打开UE编辑器
```

---

### 步骤2：配置BP_GaiaUIPolicy蓝图

1. 打开 `BP_GaiaUIPolicy`
2. 在Details面板找到 **UI → Layout Class**
3. 设置为：`WBP_GaiaPrimaryGameLayout`
4. 编译并保存

**关键：这一步是必须的！** ⚠️

---

### 步骤3：配置Project Settings

**Edit → Project Settings → Game → Game UI Manager Subsystem**

设置：
- **Default UI Policy Class** = `BP_GaiaUIPolicy`

**或者手动编辑 `Config/DefaultGame.ini`：**

```ini
[/Script/CommonGame.GameUIManagerSubsystem]
DefaultUIPolicyClass=/Game/UI/BP_GaiaUIPolicy.BP_GaiaUIPolicy_C
```

---

### 步骤4：配置GameViewportClient

**Edit → Project Settings → Engine → General Settings**

找到：**Game Viewport Client Class**

设置为：`CommonGameViewportClient`

**或者手动编辑 `Config/DefaultEngine.ini`：**

```ini
[/Script/Engine.Engine]
GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient
```

---

### 步骤5：确保使用CommonLocalPlayer

**Edit → Project Settings → Engine → General Settings**

找到：**Local Player Class**

设置为：`CommonLocalPlayer`

**或者手动编辑 `Config/DefaultEngine.ini`：**

```ini
[/Script/Engine.Engine]
LocalPlayerClassName=/Script/CommonUI.CommonLocalPlayer
```

---

### 步骤6：验证WBP_GaiaPrimaryGameLayout

打开 `WBP_GaiaPrimaryGameLayout`，确认：

1. ✅ **Parent Class = GaiaPrimaryGameLayout**（Class Settings）
2. ✅ 有4个 `CommonActivatableWidgetStack`：
   - GameLayer ✅ Is Variable
   - ContainerLayer ✅ Is Variable
   - MenuLayer ✅ Is Variable
   - ModalLayer ✅ Is Variable
3. ✅ 编译无错误

---

### 步骤7：重启编辑器

配置修改后，**必须重启UE编辑器**！

```
1. 保存所有蓝图
2. 关闭UE编辑器
3. 重新打开项目
```

---

## 🔍 验证是否修复

### 查看日志

运行PIE后，查看Output Log，应该看到：

```
LogGaia: Log: [UI Policy] 使用自定义Layout类: WBP_GaiaPrimaryGameLayout_C
LogGaia: Log: [UI Policy] 玩家布局已添加到Viewport: Player=CommonLocalPlayer_0
LogGaia: Log: [Gaia UI管理器] 初始化
```

### 测试代码

在Blueprint中测试：

```
Event BeginPlay
  → Get Gaia UI Manager Subsystem
  → Print String (如果不为null，说明成功)
```

---

## 📋 完整配置清单

### Config/DefaultGame.ini

```ini
[/Script/CommonGame.GameUIManagerSubsystem]
DefaultUIPolicyClass=/Game/UI/BP_GaiaUIPolicy.BP_GaiaUIPolicy_C
```

### Config/DefaultEngine.ini

```ini
[/Script/Engine.Engine]
GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient
LocalPlayerClassName=/Script/CommonUI.CommonLocalPlayer
```

### Config/Tags/GameplayTags_UI.ini

```ini
[/Script/GameplayTags.GameplayTagsList]
GameplayTagList=(Tag="UI.Layer.Game",DevComment="游戏主界面层")
GameplayTagList=(Tag="UI.Layer.Container",DevComment="容器窗口层")
GameplayTagList=(Tag="UI.Layer.Menu",DevComment="右键菜单/上下文菜单层")
GameplayTagList=(Tag="UI.Layer.Modal",DevComment="模态对话框层（最高层）")
```

---

## 🐛 其他常见错误

### 错误：找不到CommonActivatableWidgetStack

**原因：** CommonUI插件未启用

**解决：**
1. Edit → Plugins → 搜索 "CommonUI"
2. 勾选启用
3. 重启编辑器

---

### 错误：Widget显示不出来

**原因：** Layer未正确绑定

**解决：**
1. 打开 `WBP_GaiaPrimaryGameLayout`
2. 确保4个Layer都勾选了 **"Is Variable"**
3. 重新编译保存

---

### 错误：ESC键无效

**原因：** Widget不是ActivatableWidget

**解决：**
1. 检查Widget的父类是否是 `CommonActivatableWidget`
2. 确保使用了Layer System而非直接AddToViewport

---

## 🎯 快速诊断脚本

### C++诊断代码

```cpp
void AYourActor::DiagnoseUISystem()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GameInstance为null"));
        return;
    }

    UGameUIManagerSubsystem* UIManager = GI->GetSubsystem<UGameUIManagerSubsystem>();
    if (!UIManager)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ GameUIManagerSubsystem为null"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ GameUIManagerSubsystem已创建"));

    UGameUIPolicy* Policy = UIManager->GetCurrentUIPolicy();
    if (!Policy)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ UIPolicy为null - 检查Project Settings"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ UIPolicy已创建: %s"), *Policy->GetClass()->GetName());

    UPrimaryGameLayout* Layout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(this);
    if (!Layout)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ PrimaryGameLayout为null - 检查BP_GaiaUIPolicy的Layout Class设置"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("✅ PrimaryGameLayout已创建: %s"), *Layout->GetClass()->GetName());

    UE_LOG(LogTemp, Log, TEXT("🎉 UI系统全部正常！"));
}
```

---

## 📞 获取帮助

如果以上步骤都无法解决问题：

1. **检查UE版本** - 本系统针对UE 5.2+优化
2. **查看完整日志** - 复制完整的Output Log
3. **截图配置** - 截图Project Settings和蓝图配置
4. **检查插件** - 确认CommonUI插件版本

---

## 🔗 相关文档

- **[UI系统使用手册.md](UI系统使用手册.md)** - 完整配置步骤
- **[UI系统架构说明.md](UI系统架构说明.md)** - 深入了解原理
- **[README.md](README.md)** - 快速开始

---

**祝你顺利！** 如果还有问题，请提供详细的错误日志。

