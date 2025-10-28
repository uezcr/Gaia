# 快速修复：RootViewportLayouts为空

> **问题：** `PrimaryGameLayout` 为null，`RootViewportLayouts` 为空
> **原因：** 没有使用 `CommonGameInstance`

---

## 🚀 快速修复（3步）

### 步骤1：编译新代码 ⭐

我刚刚创建了 `UGaiaGameInstance`，**必须先编译**！

1. 关闭UE编辑器
2. 在VS/Rider中编译项目
3. 重新打开UE编辑器

---

### 步骤2：配置Project Settings ⭐⭐⭐

**Edit → Project Settings → Engine → General Settings**

找到 **Game Instance**：
- **Game Instance Class** = `GaiaGameInstance`

**或者手动编辑 `Config/DefaultEngine.ini`：**

```ini
[/Script/EngineSettings.GameMapsSettings]
GameInstanceClass=/Script/Gaia.GaiaGameInstance
```

---

### 步骤3：配置BP_GaiaUIPolicy（如果还没做）

1. 打开 `BP_GaiaUIPolicy`
2. Details面板 → **Layout Class** = `WBP_GaiaPrimaryGameLayout`
3. 编译保存

---

### 步骤4：重启编辑器

**必须重启！**

---

## ✅ 验证

运行PIE，查看Output Log，应该看到：

```
LogCommonGame: Log: AddLocalPlayer: Set CommonLocalPlayer_0 to Primary Player
LogGaia: Log: [GameInstance] Gaia游戏实例初始化
LogGaia: Log: [Gaia UI管理器] 初始化
LogGaia: Log: [UI Policy] ✅ 玩家布局已添加到Viewport
```

**不应该再看到：**
```
❌ LogGaia: Error: [Gaia UI] 无法获取PrimaryGameLayout
```

---

## 📋 完整配置清单

### Config/DefaultEngine.ini

```ini
[/Script/EngineSettings.GameMapsSettings]
GameInstanceClass=/Script/Gaia.GaiaGameInstance

[/Script/Engine.Engine]
GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient
LocalPlayerClassName=/Script/CommonUI.CommonLocalPlayer
```

### Config/DefaultGame.ini

```ini
[/Script/CommonGame.GameUIManagerSubsystem]
DefaultUIPolicyClass=/Game/UI/BP_GaiaUIPolicy.BP_GaiaUIPolicy_C
```

---

## ❓ 为什么需要CommonGameInstance？

### CommonUI初始化流程

```
1. UCommonGameInstance::AddLocalPlayer()
   ↓
2. UIManager->NotifyPlayerAdded(CommonLocalPlayer)
   ↓
3. UIPolicy->NotifyPlayerAdded(LocalPlayer)
   ↓
4. UIPolicy->CreateLayoutWidget(LocalPlayer)
   ↓
5. 创建PrimaryGameLayout并添加到RootViewportLayouts
   ↓
6. AddLayoutToViewport()
```

**如果不使用CommonGameInstance：**
- ❌ 第1步不会调用NotifyPlayerAdded
- ❌ 整个流程卡在第1步
- ❌ PrimaryGameLayout永远不会被创建
- ❌ RootViewportLayouts永远是空的

---

## 🎯 关键要点

**必须使用的3个CommonUI类：**

1. ✅ `CommonGameInstance` - 触发UI初始化
2. ✅ `CommonLocalPlayer` - 玩家管理
3. ✅ `CommonGameViewportClient` - 视口管理

**缺一不可！**

---

详细说明请查看：**[CommonUI完整配置指南.md](CommonUI完整配置指南.md)**

