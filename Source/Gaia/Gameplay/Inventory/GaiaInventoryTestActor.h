#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GaiaInventoryTestActor.generated.h"

/**
 * 库存系统单机测试Actor（重构后版本）
 * 
 * 测试重点：
 * 1. 基础功能测试
 * 2. 指针版本API测试
 * 3. 容器嵌套测试
 * 4. 物品移动测试（各种场景）
 * 5. 性能测试
 * 
 * 使用方法：
 * - 在关卡中放置此Actor
 * - 设置测试选项
 * - 点击Play或手动调用测试函数
 */
UCLASS()
class GAIAGAME_API AGaiaInventoryTestActor : public AActor
{
	GENERATED_BODY()

public:
	AGaiaInventoryTestActor();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// 测试控制
	// ========================================
	
	/** 是否在BeginPlay时自动运行测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Control")
	bool bAutoRunTests = false;

	/** 测试延迟（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Control", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float TestStartDelay = 0.5f;

	/** 是否启用详细日志 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Control")
	bool bVerboseLogging = true;

	// ========================================
	// 测试分类开关
	// ========================================
	
	/** 基础功能测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestBasicFunctions = true;

	/** 物品移动测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestItemMovement = true;

	/** 容器嵌套测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestNestedContainers = true;

	/** 堆叠测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestStacking = true;

	/** 边界情况测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestEdgeCases = true;

	/** 性能测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Categories")
	bool bTestPerformance = false;

	// ========================================
	// 性能测试设置
	// ========================================
	
	/** 性能测试：物品数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings", meta = (EditCondition = "bTestPerformance", ClampMin = "10", ClampMax = "10000"))
	int32 PerformanceTestItemCount = 1000;

	/** 性能测试：容器数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings", meta = (EditCondition = "bTestPerformance", ClampMin = "1", ClampMax = "100"))
	int32 PerformanceTestContainerCount = 10;

	// ========================================
	// 手动测试函数
	// ========================================
	
	/** 运行所有测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunAllTests();

	/** 运行基础功能测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test|Basic")
	void RunBasicFunctionTests();

	/** 运行物品移动测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test|Movement")
	void RunItemMovementTests();

	/** 运行容器嵌套测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test|Nested")
	void RunNestedContainerTests();

	/** 运行堆叠测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test|Stacking")
	void RunStackingTests();

	/** 运行边界情况测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test|EdgeCases")
	void RunEdgeCaseTests();

	/** 运行性能测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test|Performance")
	void RunPerformanceTests();

protected:
	// ========================================
	// 基础功能测试
	// ========================================
	
	/** 测试：创建容器和物品 */
	bool Test_CreateContainerAndItems();

	/** 测试：添加物品到容器 */
	bool Test_AddItemsToContainer();

	/** 测试：查找物品 */
	bool Test_FindItems();

	/** 测试：移除物品 */
	bool Test_RemoveItems();

	/** 测试：删除物品 */
	bool Test_DestroyItems();

	/** 测试：数据完整性验证 */
	bool Test_DataIntegrity();

	// ========================================
	// 物品移动测试
	// ========================================
	
	/** 测试：容器内移动 */
	bool Test_MoveWithinContainer();

	/** 测试：跨容器移动到空槽位 */
	bool Test_MoveToCrossContainerEmptySlot();

	/** 测试：跨容器移动（自动分配槽位） */
	bool Test_MoveCrossContainerAutoSlot();

	/** 测试：物品交换 */
	bool Test_SwapItems();

	/** 测试：部分移动 */
	bool Test_PartialMove();

	/** 测试：快速移动 */
	bool Test_QuickMove();

	/** 测试：拆分移动 */
	bool Test_SplitMove();

	// ========================================
	// 容器嵌套测试
	// ========================================
	
	/** 测试：放入嵌套容器 */
	bool Test_MoveToNestedContainer();

	/** 测试：多层嵌套 */
	bool Test_MultiLevelNesting();

	/** 测试：循环引用检测 */
	bool Test_CycleDetection();

	// ========================================
	// 堆叠测试
	// ========================================
	
	/** 测试：堆叠相同物品 */
	bool Test_StackSameItems();

	/** 测试：部分堆叠 */
	bool Test_PartialStacking();

	/** 测试：堆叠限制 */
	bool Test_StackLimit();

	// ========================================
	// 边界情况测试
	// ========================================
	
	/** 测试：空容器操作 */
	bool Test_EmptyContainerOperations();

	/** 测试：满容器操作 */
	bool Test_FullContainerOperations();

	/** 测试：无效UID */
	bool Test_InvalidUIDs();

	/** 测试：容器体积限制 */
	bool Test_ContainerVolumeLimit();

	/** 测试：容器标签限制 */
	bool Test_ContainerTagRestrictions();

	// ========================================
	// 性能测试
	// ========================================
	
	/** 性能测试：批量创建 */
	bool Test_Performance_BulkCreation();

	/** 性能测试：批量移动 */
	bool Test_Performance_BulkMovement();

	/** 性能测试：查找效率 */
	bool Test_Performance_SearchEfficiency();

	// ========================================
	// 辅助函数
	// ========================================
	
	/** 打印测试结果 */
	void LogTestResult(const FString& TestName, bool bSuccess, const FString& Details = TEXT(""));

	/** 打印分隔线 */
	void LogSeparator(const FString& Title = TEXT(""));

	/** 延迟执行测试 */
	void StartTestsWithDelay();

	/** 清理测试数据 */
	void CleanupTestData();

	/** 验证移动结果 */
	bool VerifyMoveResult(const struct FMoveItemResult& Result, bool bExpectedSuccess, const FString& TestContext);

	/** 验证添加结果 */
	bool VerifyAddResult(const struct FAddItemResult& Result, bool bExpectedSuccess, const FString& TestContext);

private:
	/** 测试统计 */
	int32 TotalTests = 0;
	int32 PassedTests = 0;
	int32 FailedTests = 0;

	/** 测试用的UID缓存 */
	TArray<FGuid> TestItemUIDs;
	TArray<FGuid> TestContainerUIDs;

	/** 定时器句柄 */
	FTimerHandle TestStartTimerHandle;
};
