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
	
	//~BEGIN 静态辅助函数
	
	/** 获取库存子系统实例 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory", meta = (WorldContext = "WorldContextObject"))
	static UE_API UGaiaInventorySubsystem* Get(const UObject* WorldContextObject);
	
	//~END 静态辅助函数
	
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
	UE_API FGaiaItemInstance CreateItemInstance(FName ItemDefID, int32 Quantity = 1);
	
	/** 创建容器实例 */
	UE_API FGuid CreateContainerInstance(FName ContainerDefID);
	
	//~END 实例创建
	
	//~BEGIN 调试辅助
	
	/** 设置物品的调试显示名称 */
	UE_API void SetItemDebugName(const FGuid& ItemUID, const FString& DebugName);
	
	/** 设置容器的调试显示名称 */
	UE_API void SetContainerDebugName(const FGuid& ContainerUID, const FString& DebugName);
	
	//~END 调试辅助

	//~BEGIN 网络/多人游戏支持
	
	/**
	 * 广播容器更新给所有相关玩家
	 * 通知所有客户端刷新数据
	 * @param ContainerUID 发生变化的容器UID
	 */
	UE_API void BroadcastContainerUpdate(const FGuid& ContainerUID);

	/**
	 * 获取容器的调试信息（用于UI显示）
	 * @param ContainerUID 容器UID
	 * @return 容器调试信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Debug")
	UE_API FContainerUIDebugInfo GetContainerDebugInfo(const FGuid& ContainerUID);
	
	//~END 网络/多人游戏支持

	//~BEGIN 查询功能
	
	/** 通过UID查找物品实例 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API bool FindItemByUID(const FGuid& InstanceUID, FGaiaItemInstance& OutItem);
	
	/** 通过UID查找容器实例 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API bool FindContainerByUID(const FGuid& ContainerUID, FGaiaContainerInstance& OutContainer);
	
	//~END 查询功能

	//~BEGIN 容器操作
	
	/** 
	 * 尝试添加物品到容器（带详细检查和错误信息）
	 * @param ItemUID 要添加的物品UID
	 * @param ContainerUID 目标容器UID
	 * @return 添加结果（包含成功/失败、错误信息、槽位ID等）
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API FAddItemResult TryAddItemToContainer(const FGuid& ItemUID, const FGuid& ContainerUID);
	
	/** 
     * 尝试移动物品到指定容器和槽位（带检查）
     * @param ItemUID 要移动的物品UID
     * @param TargetContainerUID 目标容器UID
     * @param TargetSlotID 目标槽位ID（-1表示自动分配）
     * @param Quantity 移动数量（-1表示全部移动）
     * @return 移动结果（包含成功/失败、错误信息等）
     */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API FMoveItemResult TryMoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID = -1, int32 Quantity = -1);

	/** 快速移动物品（移动全部数量） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API FMoveItemResult QuickMoveItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 TargetSlotID = -1);
	
	/** 拆分移动物品（移动指定数量） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API FMoveItemResult SplitItem(const FGuid& ItemUID, const FGuid& TargetContainerUID, int32 Quantity, int32 TargetSlotID = -1);

	/** 从容器移除物品（设为游离状态，不删除物品） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API bool RemoveItemFromContainer(const FGuid& ItemUID);
	
	/** 删除物品（完全从系统中移除） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	UE_API bool DestroyItem(const FGuid& ItemUID);

	//~END 容器操作

	//~BEGIN 数据验证
	
	/** 验证数据一致性（物品位置和槽位引用是否匹配） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Debug")
	UE_API bool ValidateDataIntegrity() const;
	
	/** 修复数据不一致（以槽位为准） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Debug")
	UE_API void RepairDataIntegrity();
	
	//~END 数据验证

	/** 获取物品总体积（含内容物） */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	static UE_API int32 GetItemTotalVolume(const FGaiaItemInstance& Item);
	
	/** 获取物品总重量（含内容物） */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API int32 GetItemTotalWeight(const FGaiaItemInstance& Item) const;
	
	/** 获取容器已使用体积 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API int32 GetContainerUsedVolume(const FGuid& ContainerUID) const;
	
	/** 获取容器已使用重量 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API int32 GetContainerUsedWeight(const FGuid& ContainerUID) const;

private:
	//~BEGIN 容器操作辅助函数
	
	/** 
     * 检查是否可以添加物品到容器（优化版本 - 直接使用指针）
     * @param Item 物品指针（非空，已验证）
     * @param Container 容器指针（非空，已验证）
     * @return 是否可以添加
     */
	UE_API FAddItemResult CanAddItemToContainer(FGaiaItemInstance* Item, FGaiaContainerInstance* Container) const;
	
	/** 
	 * 添加物品到容器（内部函数，不做检查，直接操作指针）
	 * @warning 仅在 TryAddItemToContainer 内部调用，已通过所有检查
	 * @param Item 物品指针（非空，已验证）
	 * @param Container 容器指针（非空，已验证）
	 * @param SlotID 目标槽位
	 * @return 是否成功添加
	 */
	UE_API bool AddItemToContainer(FGaiaItemInstance* Item, FGaiaContainerInstance* Container, const int32 SlotID = INDEX_NONE);

	/** 
     * 检查是否可以交换物品（优化版本 - 直接使用指针）
     * @param Item1 第一个物品指针（非空，已验证）
     * @param Item2 第二个物品指针（非空，已验证）
     * @return 交换检查结果（包含详细错误信息）
     */
	UE_API FMoveItemResult CanSwapItems(FGaiaItemInstance* Item1, FGaiaItemInstance* Item2) const;
	
	/** 
	 * 交换两个物品的位置（内部函数，不做检查，直接操作指针）
	 * @warning 仅在 TrySwapItems 内部调用，已通过所有检查
	 * @param Item1 第一个物品指针（非空，已验证）
	 * @param Item2 第二个物品指针（非空，已验证）
	 * @return 是否成功交换
	 */
	UE_API bool SwapItems(FGaiaItemInstance* Item1, FGaiaItemInstance* Item2);

	/** 
	 * 堆叠物品（内部函数，不做检查，直接操作指针）
	 * @warning 仅在内部调用，已通过所有检查
	 * @param SourceItem 源物品指针（非空，已验证）
	 * @param TargetItem 目标物品指针（非空，已验证）
	 * @param Quantity 堆叠数量（已验证）
	 * @return 堆叠结果
	 */
	UE_API FMoveItemResult StackItems(FGaiaItemInstance* SourceItem, FGaiaItemInstance* TargetItem, int32 Quantity);

	/** 
	 * 移动物品到另一个物品的容器中（内部函数，不做检查，直接操作指针）
	 * @warning 仅在内部调用，已通过所有检查
	 * @param Item 要移动的物品指针（非空，已验证）
	 * @param ContainerItem 容器物品指针（非空，已验证，HasContainer()为true）
	 * @param Quantity 移动数量（已验证）
	 * @return 移动结果
	 */
	UE_API FMoveItemResult MoveToItemContainer(FGaiaItemInstance* Item, FGaiaItemInstance* ContainerItem, int32 Quantity);

	/** 
     * 移动物品到指定容器和槽位（内部函数，不做检查，只做路由）
     * @warning 仅在 TryMoveItem 内部调用，已通过所有检查
     * @param Item 要移动的物品指针（非空，已验证）
     * @param TargetContainer 目标容器指针（非空，已验证）
     * @param TargetSlotID 目标槽位ID（-1表示自动分配）
     * @param Quantity 移动数量（已验证）
     * @return 移动结果
     */
	UE_API FMoveItemResult MoveItem(FGaiaItemInstance* Item, FGaiaContainerInstance* TargetContainer, int32 TargetSlotID, int32 Quantity);

	/** 
	 * 直接移动物品到空槽位（内部函数，不做检查）
	 * @warning 仅在 MoveItem 内部调用，已通过所有检查
	 * @param Item 源物品指针（非空，已验证）
	 * @param TargetContainer 目标容器指针（非空，已验证）
	 * @param TargetSlotID 目标槽位ID（已验证有效且为空）
	 * @param Quantity 移动数量（已验证）
	 * @return 移动结果
	 */
	UE_API FMoveItemResult MoveToEmptySlot(FGaiaItemInstance* Item, FGaiaContainerInstance* TargetContainer, int32 TargetSlotID, int32 Quantity);
	
	/** 
	 * 容器内移动物品（内部函数，不做检查，直接操作指针）
	 * @warning 仅在内部调用，已通过所有检查
	 * @param Item 源物品指针（非空，已验证，在容器中）
	 * @param TargetSlotID 目标槽位ID（已验证有效）
	 * @param Quantity 移动数量（已验证）
	 * @return 移动结果
	 */
	UE_API FMoveItemResult MoveItemWithinContainer(FGaiaItemInstance* Item, int32 TargetSlotID, int32 Quantity);
	
	/** 
	 * 处理目标槽位有物品的情况（内部函数，不做检查，直接操作指针）
	 * @warning 仅在内部调用，已通过所有检查
	 * @param SourceItem 源物品指针（非空，已验证）
	 * @param TargetItem 目标物品指针（非空，已验证）
	 * @param TargetContainer 目标容器指针（非空，已验证）
	 * @param TargetSlotID 目标槽位ID（已验证有效）
	 * @param Quantity 移动数量（已验证）
	 * @return 移动结果
	 */
	UE_API FMoveItemResult ProcessTargetSlotWithItem(FGaiaItemInstance* SourceItem, FGaiaItemInstance* TargetItem, FGaiaContainerInstance* TargetContainer, int32 TargetSlotID, int32 Quantity);

	/** 
     * 自动分配槽位移动（内部函数，不做检查，直接操作指针）
     * @warning 仅在内部调用，已通过所有检查
     * @param Item 源物品指针（非空，已验证）
     * @param TargetContainer 目标容器指针（非空，已验证）
     * @param Quantity 移动数量（已验证）
     * @return 移动结果
     */
	UE_API FMoveItemResult MoveItemAutoSlot(FGaiaItemInstance* Item, FGaiaContainerInstance* TargetContainer, int32 Quantity);

	//~END 容器操作辅助函数

	/** 检查是否会造成循环引用 */
	UE_API bool WouldCreateCycle(const FGuid& ItemContainerUID, const FGuid& TargetContainerUID) const;

public:
	//~BEGIN 查询辅助
	
	/** 获取容器中的所有物品 */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API TArray<FGaiaItemInstance> GetItemsInContainer(const FGuid& ContainerUID) const;
	
	/** 获取所有游离物品（不在任何容器中） */
	UFUNCTION(BlueprintPure, Category = "Gaia|Inventory")
	UE_API TArray<FGaiaItemInstance> GetOrphanItems() const;

	/** 获取所有容器 */
	UE_API void GetAllContainers(TArray<FGaiaContainerInstance>& OutContainers) const;
	
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
};

#undef UE_API

