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
	/** 是否在BeginPlay时自动运行测试 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings")
	bool bAutoRunTests = true;

	/** 性能测试的物品数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Settings", meta = (ClampMin = "10", ClampMax = "10000"))
	int32 PerformanceTestItemCount = 100;

	/** 手动触发测试（可在蓝图中调用） */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunTests();

	/** 单独运行容器测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunContainerTest();

	/** 单独运行物品测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunItemTest();

	/** 单独运行添加/查找测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunAddFindTest();

	/** 单独运行删除测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunRemoveTest();

	/** 单独运行性能测试 */
	UFUNCTION(BlueprintCallable, Category = "Gaia|Test")
	void RunPerformanceTest();
};

