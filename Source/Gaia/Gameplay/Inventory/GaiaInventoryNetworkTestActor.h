// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GaiaInventoryNetworkTestActor.generated.h"

class UGaiaInventorySubsystem;
class UGaiaInventoryRPCComponent;

/**
 * 库存系统网络测试Actor
 * 
 * 用于测试多人游戏中的库存同步功能
 * 
 * 使用方法：
 * 1. 在关卡中放置此Actor
 * 2. 设置 PIE 为 2 个玩家（Listen Server）
 * 3. 点击 Play
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
	// 测试选项
	// ========================================
	
	/** 测试1: RPC组件初始化 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Options")
	bool bTestRPCComponent = true;

	/** 测试2: 容器独占访问 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Options")
	bool bTestExclusiveAccess = true;

	/** 测试3: 物品移动同步 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Options")
	bool bTestItemMovement = true;

	/** 测试4: 服务器广播 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Options")
	bool bTestBroadcast = true;

	/** 测试5: 容器所有者注册 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Options")
	bool bTestContainerOwnership = true;

	// ========================================
	// 手动测试函数
	// ========================================
	
	/** 手动运行所有测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Network Test")
	void RunAllTests();

	/** 手动运行选中的测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Network Test")
	void RunSelectedTests();

protected:
	// ========================================
	// 内部测试函数
	// ========================================
	
	/** 测试1: RPC组件初始化 */
	bool Test_RPCComponentInitialization();

	/** 测试2: 容器独占访问 */
	bool Test_ExclusiveContainerAccess();

	/** 测试3: 物品移动同步 */
	bool Test_ItemMovementSync();

	/** 测试4: 服务器广播 */
	bool Test_ServerBroadcast();

	/** 测试5: 容器所有者注册 */
	bool Test_ContainerOwnership();

	// ========================================
	// 辅助函数
	// ========================================
	
	/** 获取所有玩家控制器 */
	TArray<APlayerController*> GetAllPlayerControllers() const;

	/** 打印测试结果 */
	void LogTestResult(const FString& TestName, bool bSuccess, const FString& Details = TEXT(""));

	/** 打印分隔线 */
	void LogSeparator(const FString& Title = TEXT(""));

	/** 延迟执行测试 */
	void StartTestsWithDelay();

private:
	/** 定时器句柄 */
	FTimerHandle TestStartTimerHandle;
};

