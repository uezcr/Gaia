// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GaiaInventoryNetworkTestActor.generated.h"

class UGaiaInventorySubsystem;
class UGaiaInventoryRPCComponent;

/**
 * 库存系统网络测试Actor（重构后版本）
 * 
 * 测试重点：
 * 1. RPC组件功能
 * 2. 容器数据同步
 * 3. 物品移动同步
 * 4. 嵌套容器同步
 * 5. 服务器-客户端一致性
 * 
 * 使用方法：
 * 1. 在关卡中放置此Actor
 * 2. 设置PIE为2个玩家（Listen Server）
 * 3. 点击Play
 * 4. 观察输出日志
 */
UCLASS()
class GAIAGAME_API AGaiaInventoryNetworkTestActor : public AActor
{
	GENERATED_BODY()

public:
	AGaiaInventoryNetworkTestActor();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// 测试控制
	// ========================================
	
	/** 是否自动运行测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Control")
	bool bAutoRunTests = true;

	/** 延迟多少秒后开始测试（等待客户端连接） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Control", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float TestStartDelay = 2.0f;

	/** 是否启用详细日志 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Control")
	bool bVerboseLogging = true;

	// ========================================
	// 测试分类开关
	// ========================================
	
	/** RPC组件测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestRPCComponent = true;

	/** 容器同步测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestContainerSync = true;

	/** 物品移动同步测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestMovementSync = true;

	/** 嵌套容器同步测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestNestedSync = true;

	/** 服务器广播测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestBroadcast = true;

	// ========================================
	// 手动测试函数
	// ========================================
	
	/** 运行所有测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Network Test")
	void RunAllTests();

	/** 运行RPC组件测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Network Test|RPC")
	void RunRPCComponentTests();

	/** 运行容器同步测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Network Test|Sync")
	void RunContainerSyncTests();

	/** 运行物品移动同步测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Network Test|Movement")
	void RunMovementSyncTests();

	/** 运行嵌套容器同步测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Network Test|Nested")
	void RunNestedSyncTests();

	/** 运行服务器广播测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Network Test|Broadcast")
	void RunBroadcastTests();

protected:
	// ========================================
	// RPC组件测试
	// ========================================
	
	/** 测试：RPC组件初始化 */
	bool Test_RPCComponentInitialization();

	/** 测试：容器注册 */
	bool Test_ContainerRegistration();

	/** 测试：数据请求刷新 */
	bool Test_DataRefresh();

	// ========================================
	// 容器同步测试
	// ========================================
	
	/** 测试：容器创建同步 */
	bool Test_ContainerCreationSync();

	/** 测试：物品添加同步 */
	bool Test_ItemAdditionSync();

	/** 测试：物品移除同步 */
	bool Test_ItemRemovalSync();

	/** 测试：容器数据一致性 */
	bool Test_ContainerDataConsistency();

	// ========================================
	// 物品移动同步测试
	// ========================================
	
	/** 测试：容器内移动同步 */
	bool Test_WithinContainerMoveSync();

	/** 测试：跨容器移动同步 */
	bool Test_CrossContainerMoveSync();

	/** 测试：物品交换同步 */
	bool Test_ItemSwapSync();

	/** 测试：堆叠同步 */
	bool Test_StackingSync();

	// ========================================
	// 嵌套容器同步测试
	// ========================================
	
	/** 测试：嵌套容器自动同步 */
	bool Test_NestedContainerAutoSync();

	/** 测试：多层嵌套同步 */
	bool Test_MultiLevelNestedSync();

	/** 测试：嵌套容器移动同步 */
	bool Test_NestedContainerMoveSync();

	// ========================================
	// 服务器广播测试
	// ========================================
	
	/** 测试：服务器广播物品变化 */
	bool Test_BroadcastItemChanges();

	/** 测试：多客户端同步 */
	bool Test_MultiClientSync();

	// ========================================
	// 辅助函数
	// ========================================
	
	/** 获取所有玩家控制器 */
	TArray<APlayerController*> GetAllPlayerControllers() const;

	/** 获取服务器玩家控制器 */
	APlayerController* GetServerPlayerController() const;

	/** 获取客户端玩家控制器 */
	TArray<APlayerController*> GetClientPlayerControllers() const;

	/** 获取指定玩家的RPC组件 */
	UGaiaInventoryRPCComponent* GetRPCComponent(APlayerController* PC) const;

	/** 打印测试结果 */
	void LogTestResult(const FString& TestName, bool bSuccess, const FString& Details = TEXT(""));

	/** 打印分隔线 */
	void LogSeparator(const FString& Title = TEXT(""));

	/** 延迟执行测试 */
	void StartTestsWithDelay();

	/** 验证容器数据在客户端和服务器上一致 */
	bool VerifyContainerConsistency(const FGuid& ContainerUID, const FString& TestContext);

	/** 打印网络状态 */
	void LogNetworkStatus();

private:
	/** 测试统计 */
	int32 TotalTests = 0;
	int32 PassedTests = 0;
	int32 FailedTests = 0;

	/** 定时器句柄 */
	FTimerHandle TestStartTimerHandle;

	/** 测试用的UID缓存 */
	TArray<FGuid> TestItemUIDs;
	TArray<FGuid> TestContainerUIDs;
};
