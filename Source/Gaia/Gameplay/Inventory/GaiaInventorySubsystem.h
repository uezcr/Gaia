#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "GaiaInventoryTypes.h"
#include "GaiaInventorySubsystem.generated.h"

#define UE_API GAIAGAME_API

/**
 * Gaia库存管理器设置
 * 用于配置库存系统的全局参数
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Gaia Inventory Manager"), MinimalAPI)
class UGaiaInventoryManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGaiaInventoryManagerSettings() {}
	
	/** 物品定义数据注册表类型 */
	UPROPERTY(config, EditAnywhere, Category = "Data Registry")
	FName ItemDefinitionRegistryType;
	
	/** 容器定义数据注册表类型 */
	UPROPERTY(config, EditAnywhere, Category = "Data Registry")
	FName ContainerDefinitionRegistryType;
};

/**
 * Gaia库存子系统
 * 管理全局的物品和容器数据
 * 继承自UWorldSubsystem，每个World有独立的实例
 */
UCLASS(MinimalAPI)
class UGaiaInventorySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UE_API UGaiaInventorySubsystem();

	//~ Begin USubsystem Interface
	UE_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UE_API virtual void Deinitialize() override;
	//~ End USubsystem Interface

public:
	//~BEGIN 静态辅助函数
	
	/** 获取库存子系统实例 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory", meta = (WorldContext = "WorldContextObject"))
	static UE_API UGaiaInventorySubsystem* Get(const UObject* WorldContextObject);
	
	//~END 静态辅助函数

public:
	//~BEGIN 数据定义获取
	
	/** 获取物品定义 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	static UE_API bool GetItemDefinition(FName ItemDefID, FGaiaItemDefinition& OutItemDef);
	
	/** 获取容器定义 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	static UE_API bool GetContainerDefinition(FName ContainerDefID, FGaiaContainerDefinition& OutContainerDef);
	
	//~END 数据定义获取

	//~BEGIN 实例创建
	
	/** 创建物品实例 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API FGaiaItemInstance CreateItemInstance(FName ItemDefID, int32 Quantity = 1);
	
	/** 创建容器实例 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API FGuid CreateContainer(FName ContainerDefID);
	
	//~END 实例创建

	//~BEGIN 查询功能
	
	/** 通过UID查找物品实例 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API bool FindItemByUID(const FGuid& InstanceUID, FGaiaItemInstance& OutItem);
	
	/** 通过UID查找容器实例 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API bool FindContainerByUID(const FGuid& ContainerUID, FGaiaContainerInstance& OutContainer);
	
	//~END 查询功能

	//~BEGIN 容器操作
	
	/** 添加物品到容器 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API bool AddItemToContainer(const FGaiaItemInstance& Item, const FGuid& ContainerUID);
	
	/** 从容器移除物品（设为游离状态，不删除物品） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API bool RemoveItemFromContainer(const FGuid& ItemUID);
	
	/** 删除物品（完全从系统中移除） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API bool DestroyItem(const FGuid& ItemUID);
	
	/** 检查是否可以添加物品到容器 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API bool CanAddItemToContainer(const FGaiaItemInstance& Item, const FGuid& ContainerUID) const;
	
	//~END 容器操作

	//~BEGIN 物品移动
	
	/** 移动物品到指定容器和槽位 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API FMoveItemResult MoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID = -1, int32 Quantity = -1);
	
	/** 检查是否可以移动物品到指定位置 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API bool CanMoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID = -1, int32 Quantity = -1);
	
	/** 快速移动物品（移动全部数量） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API FMoveItemResult QuickMoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID = -1);
	
	/** 拆分移动物品（移动指定数量） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API FMoveItemResult SplitItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 Quantity, int32 TargetSlotID = -1);
	
	//~END 物品移动

	//~BEGIN 嵌套检测
	
	/** 检查是否会造成循环引用 */
	UE_API bool WouldCreateCycle(const FGuid& ItemContainerUID, const FGuid& TargetContainerUID) const;
	
	//~END 嵌套检测

	//~BEGIN 体积/重量计算
	
	/** 获取物品总体积（含内容物） */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API int32 GetItemTotalVolume(const FGaiaItemInstance& Item) const;
	
	/** 获取物品总重量（含内容物） */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API int32 GetItemTotalWeight(const FGaiaItemInstance& Item) const;
	
	/** 获取容器已使用体积 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API int32 GetContainerUsedVolume(const FGuid& ContainerUID) const;
	
	/** 获取容器已使用重量 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API int32 GetContainerUsedWeight(const FGuid& ContainerUID) const;
	
	//~END 体积/重量计算

	//~BEGIN 数据验证
	
	/** 验证数据一致性（物品位置和槽位引用是否匹配） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Debug")
	UE_API bool ValidateDataIntegrity() const;
	
	/** 修复数据不一致（以槽位为准） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Debug")
	UE_API void RepairDataIntegrity();
	
	//~END 数据验证

	//~BEGIN 查询辅助
	
	/** 获取容器中的所有物品 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API TArray<FGaiaItemInstance> GetItemsInContainer(const FGuid& ContainerUID) const;
	
	/** 获取所有游离物品（不在任何容器中） */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API TArray<FGaiaItemInstance> GetOrphanItems() const;
	
	/** 统计全局指定类型物品的总数量 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API int32 CountItemsByType(FName ItemDefID) const;
	
	//~END 查询辅助

private:
	/** 所有物品实例的映射表（FGuid -> 物品实例） - 单一数据源 */
	UPROPERTY()
	TMap<FGuid, FGaiaItemInstance> AllItems;
	
	/** 所有容器实例的映射表（FGuid -> 容器实例） */
	UPROPERTY()
	TMap<FGuid, FGaiaContainerInstance> Containers;
	
	//~BEGIN 移动辅助函数
	
	/** 尝试堆叠物品 */
	FMoveItemResult TryStackItems(const FGuid& SourceItemUID, const FGuid& TargetItemUID, int32 Quantity);
	
	/** 尝试将物品放入目标物品的容器 */
	FMoveItemResult TryMoveToContainer(const FGuid& ItemUID, const FGuid& ContainerItemUID, int32 Quantity);
	
	/** 交换两个物品的位置 */
	FMoveItemResult SwapItems(const FGuid& ItemUID1, const FGuid& ItemUID2);
	
	/** 检查是否可以交换物品 */
	bool CanSwapItems(const FGuid& ItemUID1, const FGuid& ItemUID2);
	
	/** 直接移动物品到空槽位 */
	FMoveItemResult MoveToEmptySlot(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID, int32 Quantity);
	
	/** 容器内移动物品 */
	FMoveItemResult MoveItemWithinContainer(const FGuid& ItemUID, int32 TargetSlotID, int32 Quantity);
	
	/** 处理目标槽位有物品的情况 */
	FMoveItemResult ProcessTargetSlotWithItem(const FGuid& ItemUID, const FGaiaItemInstance& TargetItem, const FGuid& TargetContainerUID, int32 TargetSlotID, int32 Quantity);
	
	/** 自动分配槽位移动 */
	FMoveItemResult MoveItemAutoSlot(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 Quantity);
	
	//~END 移动辅助函数
};

#undef UE_API

