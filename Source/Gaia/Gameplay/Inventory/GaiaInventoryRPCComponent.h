// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GaiaInventoryTypes.h"
#include "GaiaInventoryRPCComponent.generated.h"

class UGaiaInventorySubsystem;

/**
 * 库存系统网络RPC组件
 * 
 * 设计理念：
 * - 轻量级RPC通道，不存储数据
 * - 所有数据存储在服务器的 UGaiaInventorySubsystem 中
 * - 客户端通过此组件发送请求，接收更新
 * 
 * 使用方式：
 * - 挂载在 PlayerController 或 PlayerState 上
 * - 自动在服务器和客户端之间同步库存操作
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class GAIAGAME_API UGaiaInventoryRPCComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGaiaInventoryRPCComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UActorComponent Interface

public:
	// ========================================
	// 客户端请求接口（蓝图可调用）
	// ========================================
	
	/**
	 * 请求移动物品
	 * @param ItemUID 要移动的物品UID
	 * @param TargetContainerUID 目标容器UID
	 * @param TargetSlotID 目标槽位ID（-1表示自动分配）
	 * @param Quantity 移动数量（0表示全部）
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	void RequestMoveItem(
		const FGuid& ItemUID,
		const FGuid& TargetContainerUID,
		int32 TargetSlotID = -1,
		int32 Quantity = 0
	);

	/**
	 * 请求添加物品到容器
	 * @param ItemUID 要添加的物品UID
	 * @param ContainerUID 目标容器UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	void RequestAddItem(
		const FGuid& ItemUID,
		const FGuid& ContainerUID
	);

	/**
	 * 请求从容器移除物品（设为游离状态）
	 * @param ItemUID 要移除的物品UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	void RequestRemoveItem(const FGuid& ItemUID);

	/**
	 * 请求销毁物品
	 * @param ItemUID 要销毁的物品UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	void RequestDestroyItem(const FGuid& ItemUID);

	/**
	 * 请求打开世界容器（箱子等）
	 * @param ContainerUID 容器UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	void RequestOpenWorldContainer(const FGuid& ContainerUID);

	/**
	 * 请求关闭世界容器
	 * @param ContainerUID 容器UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	void RequestCloseWorldContainer(const FGuid& ContainerUID);

	/**
	 * 请求刷新库存数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	void RequestRefreshInventory();

public:
	// ========================================
	// 服务器RPC
	// ========================================
	
	/** 服务器RPC：移动物品 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerMoveItem(
		const FGuid& ItemUID,
		const FGuid& TargetContainerUID,
		int32 TargetSlotID,
		int32 Quantity
	);

	/** 服务器RPC：添加物品到容器 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItem(
		const FGuid& ItemUID,
		const FGuid& ContainerUID
	);

	/** 服务器RPC：从容器移除物品 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRemoveItem(const FGuid& ItemUID);

	/** 服务器RPC：销毁物品 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDestroyItem(const FGuid& ItemUID);

	/** 服务器RPC：打开世界容器 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOpenWorldContainer(const FGuid& ContainerUID);

	/** 服务器RPC：关闭世界容器 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCloseWorldContainer(const FGuid& ContainerUID);

	/** 服务器RPC：刷新库存 */
	UFUNCTION(Server, Reliable)
	void ServerRequestRefreshInventory();

public:
	// ========================================
	// 客户端RPC
	// ========================================
	
	/**
	 * 客户端RPC：接收库存数据更新
	 * @param Items 物品列表
	 * @param Containers 容器列表
	 */
	UFUNCTION(Client, Reliable)
	void ClientReceiveInventoryData(
		const TArray<FGaiaItemInstance>& Items,
		const TArray<FGaiaContainerInstance>& Containers
	);

	/**
	 * 客户端RPC：接收增量更新
	 * @param UpdatedItems 更新的物品
	 * @param RemovedItemUIDs 移除的物品UID
	 * @param UpdatedContainers 更新的容器
	 */
	UFUNCTION(Client, Reliable)
	void ClientReceiveInventoryDelta(
		const TArray<FGaiaItemInstance>& UpdatedItems,
		const TArray<FGuid>& RemovedItemUIDs,
		const TArray<FGaiaContainerInstance>& UpdatedContainers
	);

	/**
	 * 客户端RPC：操作成功通知
	 * @param Message 成功消息
	 */
	UFUNCTION(Client, Reliable)
	void ClientOperationSuccess(const FString& Message);

	/**
	 * 客户端RPC：操作失败通知
	 * @param ErrorCode 错误代码
	 * @param ErrorMessage 错误消息
	 */
	UFUNCTION(Client, Reliable)
	void ClientOperationFailed(int32 ErrorCode, const FString& ErrorMessage);

public:
	// ========================================
	// 蓝图事件（客户端UI监听）
	// ========================================
	
	/**
	 * 库存数据已更新事件
	 * UI可以监听此事件来刷新显示
	 */
	UPROPERTY(BlueprintAssignable, Category = "Gaia|Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	/**
	 * 操作失败事件
	 * UI可以监听此事件来显示错误提示
	 */
	UPROPERTY(BlueprintAssignable, Category = "Gaia|Inventory")
	FOnInventoryOperationFailed OnOperationFailed;

	/**
	 * 容器打开事件
	 */
	UPROPERTY(BlueprintAssignable, Category = "Gaia|Inventory")
	FOnContainerOpened OnContainerOpened;

	/**
	 * 容器关闭事件
	 */
	UPROPERTY(BlueprintAssignable, Category = "Gaia|Inventory")
	FOnContainerClosed OnContainerClosed;

public:
	// ========================================
	// 客户端本地缓存（仅用于UI显示）
	// ========================================
	
	/**
	 * 获取本地缓存的物品数据
	 * 注意：这是客户端的只读副本，不要修改！
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	bool GetCachedItem(const FGuid& ItemUID, FGaiaItemInstance& OutItem) const;

	/**
	 * 获取本地缓存的容器数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	bool GetCachedContainer(const FGuid& ContainerUID, FGaiaContainerInstance& OutContainer) const;

	/**
	 * 获取玩家拥有的所有容器UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	TArray<FGuid> GetOwnedContainerUIDs() const { return OwnedContainerUIDs; }

	/**
	 * 获取当前打开的世界容器UID列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory")
	TArray<FGuid> GetOpenWorldContainerUIDs() const { return OpenWorldContainerUIDs; }

	/**
	 * 获取本地缓存的物品数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Debug")
	int32 GetCachedItemCount() const { return CachedItems.Num(); }

	/**
	 * 获取本地缓存的容器数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Debug")
	int32 GetCachedContainerCount() const { return CachedContainers.Num(); }

	/**
	 * 获取所有本地缓存的物品UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Debug")
	TArray<FGuid> GetCachedItemUIDs() const;

	/**
	 * 获取所有本地缓存的容器UID
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Debug")
	TArray<FGuid> GetCachedContainerUIDs() const;

protected:
	// ========================================
	// 内部辅助函数
	// ========================================
	
	/** 获取库存子系统 */
	UGaiaInventorySubsystem* GetInventorySubsystem();

	/** 获取所属的PlayerController */
	APlayerController* GetOwningPlayerController() const;

private:
	// ========================================
	// 客户端本地数据（仅用于UI显示）
	// ========================================
	
	/** 本地缓存的物品数据 */
	UPROPERTY()
	TMap<FGuid, FGaiaItemInstance> CachedItems;

	/** 本地缓存的容器数据 */
	UPROPERTY()
	TMap<FGuid, FGaiaContainerInstance> CachedContainers;

	/** 玩家拥有的容器UID列表（复制自服务器） */
	UPROPERTY(ReplicatedUsing=OnRep_OwnedContainers)
	TArray<FGuid> OwnedContainerUIDs;

	/** 当前打开的世界容器UID列表 */
	UPROPERTY(ReplicatedUsing=OnRep_OpenWorldContainers)
	TArray<FGuid> OpenWorldContainerUIDs;

	/** 缓存的Subsystem引用 */
	UPROPERTY()
	TObjectPtr<UGaiaInventorySubsystem> CachedSubsystem;

	// ========================================
	// 复制回调
	// ========================================
	
	UFUNCTION()
	void OnRep_OwnedContainers();

	UFUNCTION()
	void OnRep_OpenWorldContainers() const;
};

