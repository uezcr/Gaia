# Gaia UI 系统文档

本目录包含 Gaia 项目 UI 系统的所有文档。

## 📁 文档结构

### Button（按钮系统）
通用按钮组件的设计、实现和使用文档。

- **[GaiaButton使用指南.md](./Button/GaiaButton使用指南.md)** - 完整的使用指南和 API 参考
- **[GaiaButton完成总结.md](./Button/GaiaButton完成总结.md)** - 开发完成总结和设计亮点
- **[CommonButton分析与设计.md](./Button/CommonButton分析与设计.md)** - CommonUI 按钮系统分析和 GaiaButton 设计方案

### Window（窗口系统）
可拖拽窗口系统的文档，包括标题栏、窗口管理等。

- **[GaiaDraggableWindow使用指南.md](./Window/GaiaDraggableWindow使用指南.md)** - 窗口组件使用指南
- **[GaiaDraggableTitleBar使用指南.md](./Window/GaiaDraggableTitleBar使用指南.md)** - 标题栏组件使用指南
- **[Canvas窗口系统使用指南.md](./Window/Canvas窗口系统使用指南.md)** - Canvas 窗口系统架构和原理
- **[可拖拽窗口完成总结.md](./Window/可拖拽窗口完成总结.md)** - 窗口系统开发总结
- **[可拖动标题栏完成总结.md](./Window/可拖动标题栏完成总结.md)** - 标题栏系统开发总结

### Archive（调试和历史文档）
调试指南和已归档的文档。

- **[拖动问题调试指南.md](./Archive/拖动问题调试指南.md)** - 拖动功能调试问题和解决方案

---

## 🚀 快速开始

### 按钮系统
如果你想创建自定义按钮，从这里开始：
1. 阅读 [GaiaButton使用指南.md](./Button/GaiaButton使用指南.md)
2. 查看 [CommonButton分析与设计.md](./Button/CommonButton分析与设计.md) 了解设计思路

### 窗口系统
如果你想创建可拖拽窗口，从这里开始：
1. 阅读 [Canvas窗口系统使用指南.md](./Window/Canvas窗口系统使用指南.md) 了解架构
2. 阅读 [GaiaDraggableWindow使用指南.md](./Window/GaiaDraggableWindow使用指南.md) 学习使用
3. 阅读 [GaiaDraggableTitleBar使用指南.md](./Window/GaiaDraggableTitleBar使用指南.md) 了解标题栏

---

## 📊 组件概览

### 核心 UI 组件

| 组件 | 文件 | 说明 |
|------|------|------|
| **GaiaButton** | `GaiaButton.h/cpp` | 通用按钮基类，8种预设样式 |
| **GaiaDraggableWindow** | `GaiaDraggableWindow.h/cpp` | 可拖拽窗口基类 |
| **GaiaDraggableTitleBar** | `GaiaDraggableTitleBar.h/cpp` | 可拖拽标题栏组件 |
| **GaiaGameHUD** | `GaiaGameHUD.h/cpp` | 游戏 HUD 管理器 |
| **GaiaUIManagerSubsystem** | `GaiaUIManagerSubsystem.h/cpp` | UI 管理子系统 |
| **GaiaPrimaryGameLayout** | `GaiaPrimaryGameLayout.h/cpp` | 主游戏布局 |

### 库存 UI 组件

| 组件 | 文件 | 说明 |
|------|------|------|
| **GaiaItemSlotWidget** | `Inventory/GaiaItemSlotWidget.h/cpp` | 物品槽位控件 |
| **GaiaContainerGridWidget** | `Inventory/GaiaContainerGridWidget.h/cpp` | 容器网格控件 |
| **GaiaContainerWindowWidget** | `Inventory/GaiaContainerWindowWidget.h/cpp` | 容器窗口控件 |
| **GaiaItemContextMenu** | `Inventory/GaiaItemContextMenu.h/cpp` | 物品右键菜单 |
| **GaiaContextMenuButton** | `Inventory/GaiaContextMenuButton.h/cpp` | 菜单按钮 |
| **GaiaItemDragDropOperation** | `Inventory/GaiaItemDragDropOperation.h/cpp` | 物品拖放操作 |

---

## 🎯 设计原则

### 1. CommonUI 集成
所有 UI 组件都基于 CommonUI 框架构建，支持：
- ✅ 键盘导航
- ✅ 手柄输入
- ✅ 输入路由管理
- ✅ 焦点系统

### 2. 模块化设计
每个组件职责单一，易于组合和扩展：
- ✅ 标题栏独立于窗口
- ✅ 按钮样式可配置
- ✅ 容器独立于物品

### 3. 蓝图友好
所有核心功能都可在蓝图中使用：
- ✅ `UFUNCTION(BlueprintCallable)`
- ✅ 枚举使用 `UMETA(DisplayName)`
- ✅ 详细的工具提示

### 4. 性能优化
- ✅ 使用 `TObjectPtr<>` 智能指针
- ✅ 状态变化时才重绘
- ✅ 避免不必要的查找和拷贝

---

## 📝 文档编写规范

### 文档类型

1. **使用指南** - 面向开发者，详细的使用说明
   - 快速开始
   - 功能介绍
   - API 参考
   - 示例代码
   - 常见问题

2. **完成总结** - 面向团队，开发完成后的总结
   - 完成的功能
   - 设计亮点
   - 代码统计
   - 使用场景

3. **分析与设计** - 面向架构师，深度技术分析
   - 技术调研
   - 设计方案
   - 对比分析
   - 实现计划

4. **调试指南** - 面向问题排查，特定问题的解决方案
   - 问题描述
   - 诊断步骤
   - 解决方案
   - 预防措施

### 命名规范

- **使用指南**：`组件名称使用指南.md`
- **完成总结**：`组件名称完成总结.md`
- **分析设计**：`技术名称分析与设计.md`
- **调试指南**：`问题描述调试指南.md`

---

## 🔧 维护说明

### 添加新文档
1. 根据文档类型放入对应文件夹（Button/Window/Archive）
2. 更新本 README 的文档列表
3. 在组件源码中添加文档链接的注释

### 归档文档
当文档过时或问题已解决时：
1. 移动到 `Archive` 文件夹
2. 在文档开头添加归档说明
3. 从主文档列表中移除（保留在 Archive 部分）

### 更新文档
- 小改动：直接修改
- 大改动：创建新版本（文件名加版本号）

---

## 📚 相关资源

### 引擎文档
- [CommonUI Plugin](https://docs.unrealengine.com/5.3/en-US/common-ui-plugin-for-advanced-input-in-unreal-engine/)
- [UMG UI Designer](https://docs.unrealengine.com/5.3/en-US/umg-ui-designer-for-unreal-engine/)
- [Slate Architecture](https://docs.unrealengine.com/5.3/en-US/understanding-the-slate-ui-architecture/)

### 项目相关
- [库存系统文档](../../Gameplay/Inventory/README.md)
- [项目架构说明](../架构说明.md)

---

## 🤝 贡献

欢迎贡献文档改进！请遵循以下原则：
- 保持文档结构清晰
- 使用 Markdown 格式
- 添加代码示例
- 包含截图（如果适用）
- 更新 README 索引

---

**最后更新**: 2025-10-31
**维护者**: Gaia 开发团队


