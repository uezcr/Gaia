#pragma once

#include "CoreMinimal.h"
#include "GaiaInventoryTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GaiaInventoryTestHelper.generated.h"

class UGaiaInventorySubsystem;
/**
 * 库存系统测试辅助类
 * 提供蓝图可调用的测试函数
 */
UCLASS()
class GAIAGAME_API UGaiaInventoryTestHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 运行基础测试套件
	 * @param WorldContextObject 世界上下文对象
	 * @return 测试是否全部通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunBasicTests(const UObject* WorldContextObject);

	/**
	 * 测试容器创建和查找
	 * @param WorldContextObject 世界上下文对象
	 * @return 测试是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestContainerCreation(const UObject* WorldContextObject);

	/**
	 * 测试物品创建和UID唯一性
	 * @param WorldContextObject 世界上下文对象
	 * @return 测试是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestItemCreation(const UObject* WorldContextObject);

	/**
	 * 测试物品添加和查找
	 * @param WorldContextObject 世界上下文对象
	 * @return 测试是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestAddAndFindItem(const UObject* WorldContextObject);

	/**
	 * 测试物品删除和索引重建（旧架构，已废弃）
	 * @param WorldContextObject 世界上下文对象
	 * @return 测试是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestRemoveItem(const UObject* WorldContextObject);

	/**
	 * 测试物品移除和游离状态（新架构）
	 * @param WorldContextObject 世界上下文对象
	 * @return 测试是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestRemoveAndOrphanItem(const UObject* WorldContextObject);

	/**
	 * 测试数据一致性验证（新架构）
	 * @param WorldContextObject 世界上下文对象
	 * @return 测试是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestDataIntegrity(const UObject* WorldContextObject);

	/**
	 * 测试容器删除逻辑（新架构）
	 * @param WorldContextObject 世界上下文对象
	 * @return 测试是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestContainerDeletion(const UObject* WorldContextObject);

	/**
	 * 测试大量物品的性能
	 * @param WorldContextObject 世界上下文对象
	 * @param ItemCount 要创建的物品数量
	 * @return 测试是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool TestPerformance(const UObject* WorldContextObject, int32 ItemCount = 1000);

	/**
	 * 测试物品移动功能（待重构）
	 * @param WorldContextObject 世界上下文对象
	 * @return 测试是否通过
	 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Inventory|Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunMoveItemTests(const UObject* WorldContextObject);

private:
	/** 打印测试结果 */
	static void LogTestResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
	
	/** 格式化移动结果用于日志输出 */
	static FString FormatMoveResult(const FMoveItemResult& Result);
	
	/** 打印容器信息 */
	static void PrintContainerInfo(UGaiaInventorySubsystem* InventorySystem, const FGuid& ContainerUID, const FString& ContainerName);
};

