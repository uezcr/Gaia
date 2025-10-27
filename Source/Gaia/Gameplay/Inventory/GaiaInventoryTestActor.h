#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GaiaInventoryTestActor.generated.h"

/**
 * 库存系统测试Actor
 * 可以直接放置在关卡中进行测试
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
	// 测试控制开关
	// ========================================
	
	/** 是否在BeginPlay时自动运行测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Control")
	bool bAutoRunTests = false;

	/** 是否运行所有基础测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Control", meta = (EditCondition = "bAutoRunTests"))
	bool bRunAllBasicTests = true;

	// ========================================
	// 单独测试开关（当bRunAllBasicTests=false时生效）
	// ========================================
	
	/** 测试容器创建 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Tests", meta = (EditCondition = "bAutoRunTests && !bRunAllBasicTests"))
	bool bTestContainerCreation = true;

	/** 测试物品创建 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Tests", meta = (EditCondition = "bAutoRunTests && !bRunAllBasicTests"))
	bool bTestItemCreation = true;

	/** 测试物品添加和查找 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Tests", meta = (EditCondition = "bAutoRunTests && !bRunAllBasicTests"))
	bool bTestAddAndFind = true;

	/** 测试物品移除和游离状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Tests", meta = (EditCondition = "bAutoRunTests && !bRunAllBasicTests"))
	bool bTestRemoveAndOrphan = true;

	/** 测试数据完整性 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Tests", meta = (EditCondition = "bAutoRunTests && !bRunAllBasicTests"))
	bool bTestDataIntegrity = true;

	/** 测试容器删除 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Tests", meta = (EditCondition = "bAutoRunTests && !bRunAllBasicTests"))
	bool bTestContainerDeletion = true;

	/** 测试物品移动功能 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Tests", meta = (EditCondition = "bAutoRunTests && !bRunAllBasicTests"))
	bool bTestMoveItems = true;

	// ========================================
	// 性能测试设置
	// ========================================
	
	/** 是否运行性能测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Tests")
	bool bRunPerformanceTest = false;

	/** 性能测试的物品数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Tests", meta = (EditCondition = "bRunPerformanceTest", ClampMin = "10", ClampMax = "10000"))
	int32 PerformanceTestItemCount = 100;

	// ========================================
	// 手动测试函数
	// ========================================
	
	/** 手动运行所有基础测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunAllBasicTests();

	/** 手动运行选中的测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunSelectedTests();

	/** 单独运行容器测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunContainerTest();

	/** 单独运行物品测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunItemTest();

	/** 单独运行添加/查找测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunAddFindTest();

	/** 单独运行移除测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunRemoveTest();

	/** 单独运行移动测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunMoveTest();

	/** 单独运行数据完整性测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunDataIntegrityTest();

	/** 单独运行容器删除测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunContainerDeletionTest();

	/** 单独运行性能测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunPerformanceTest();
};

