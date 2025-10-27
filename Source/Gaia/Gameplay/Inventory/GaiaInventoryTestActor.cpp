#include "GaiaInventoryTestActor.h"
#include "GaiaInventoryTestHelper.h"
#include "GaiaLogChannels.h"

AGaiaInventoryTestActor::AGaiaInventoryTestActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGaiaInventoryTestActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRunTests)
	{
		// 延迟一帧执行，确保所有系统都已初始化
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			if (bRunAllBasicTests)
			{
				// 运行所有基础测试
				RunAllBasicTests();
			}
			else
			{
				// 运行选中的测试
				RunSelectedTests();
			}

			// 运行性能测试（如果启用）
			if (bRunPerformanceTest)
			{
				RunPerformanceTest();
			}
		});
	}
}

void AGaiaInventoryTestActor::RunAllBasicTests()
{
	UE_LOG(LogGaia, Warning, TEXT("========================================"));
	UE_LOG(LogGaia, Warning, TEXT("开始运行所有基础测试..."));
	UE_LOG(LogGaia, Warning, TEXT("========================================"));
	
	UGaiaInventoryTestHelper::RunBasicTests(this);
}

void AGaiaInventoryTestActor::RunSelectedTests()
{
	UE_LOG(LogGaia, Warning, TEXT("========================================"));
	UE_LOG(LogGaia, Warning, TEXT("开始运行选中的测试..."));
	UE_LOG(LogGaia, Warning, TEXT("========================================"));

	bool bAllTestsPassed = true;

	// 测试1: 容器创建
	if (bTestContainerCreation)
	{
		if (!UGaiaInventoryTestHelper::TestContainerCreation(this))
		{
			UE_LOG(LogGaia, Error, TEXT("容器创建测试失败！"));
			bAllTestsPassed = false;
		}
	}

	// 测试2: 物品创建
	if (bTestItemCreation && bAllTestsPassed)
	{
		if (!UGaiaInventoryTestHelper::TestItemCreation(this))
		{
			UE_LOG(LogGaia, Error, TEXT("物品创建测试失败！"));
			bAllTestsPassed = false;
		}
	}

	// 测试3: 物品添加和查找
	if (bTestAddAndFind && bAllTestsPassed)
	{
		if (!UGaiaInventoryTestHelper::TestAddAndFindItem(this))
		{
			UE_LOG(LogGaia, Error, TEXT("物品添加和查找测试失败！"));
			bAllTestsPassed = false;
		}
	}

	// 测试4: 物品移除和游离状态
	if (bTestRemoveAndOrphan && bAllTestsPassed)
	{
		if (!UGaiaInventoryTestHelper::TestRemoveAndOrphanItem(this))
		{
			UE_LOG(LogGaia, Error, TEXT("物品移除测试失败！"));
			bAllTestsPassed = false;
		}
	}

	// 测试5: 数据完整性
	if (bTestDataIntegrity && bAllTestsPassed)
	{
		if (!UGaiaInventoryTestHelper::TestDataIntegrity(this))
		{
			UE_LOG(LogGaia, Error, TEXT("数据完整性测试失败！"));
			bAllTestsPassed = false;
		}
	}

	// 测试6: 容器删除
	if (bTestContainerDeletion && bAllTestsPassed)
	{
		if (!UGaiaInventoryTestHelper::TestContainerDeletion(this))
		{
			UE_LOG(LogGaia, Error, TEXT("容器删除测试失败！"));
			bAllTestsPassed = false;
		}
	}

	// 测试7: 物品移动
	if (bTestMoveItems && bAllTestsPassed)
	{
		if (!UGaiaInventoryTestHelper::RunMoveItemTests(this))
		{
			UE_LOG(LogGaia, Error, TEXT("物品移动测试失败！"));
			bAllTestsPassed = false;
		}
	}

	// 输出最终结果
	UE_LOG(LogGaia, Warning, TEXT("========================================"));
	if (bAllTestsPassed)
	{
		UE_LOG(LogGaia, Warning, TEXT("所有选中的测试通过！✓"));
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("部分测试失败！✗"));
	}
	UE_LOG(LogGaia, Warning, TEXT("========================================"));
}

void AGaiaInventoryTestActor::RunContainerTest()
{
	UE_LOG(LogGaia, Log, TEXT("手动运行容器创建测试..."));
	UGaiaInventoryTestHelper::TestContainerCreation(this);
}

void AGaiaInventoryTestActor::RunItemTest()
{
	UE_LOG(LogGaia, Log, TEXT("手动运行物品创建测试..."));
	UGaiaInventoryTestHelper::TestItemCreation(this);
}

void AGaiaInventoryTestActor::RunAddFindTest()
{
	UE_LOG(LogGaia, Log, TEXT("手动运行物品添加和查找测试..."));
	UGaiaInventoryTestHelper::TestAddAndFindItem(this);
}

void AGaiaInventoryTestActor::RunRemoveTest()
{
	UE_LOG(LogGaia, Log, TEXT("手动运行物品移除测试..."));
	UGaiaInventoryTestHelper::TestRemoveAndOrphanItem(this);
}

void AGaiaInventoryTestActor::RunMoveTest()
{
	UE_LOG(LogGaia, Log, TEXT("手动运行物品移动测试..."));
	UGaiaInventoryTestHelper::RunMoveItemTests(this);
}

void AGaiaInventoryTestActor::RunDataIntegrityTest()
{
	UE_LOG(LogGaia, Log, TEXT("手动运行数据完整性测试..."));
	UGaiaInventoryTestHelper::TestDataIntegrity(this);
}

void AGaiaInventoryTestActor::RunContainerDeletionTest()
{
	UE_LOG(LogGaia, Log, TEXT("手动运行容器删除测试..."));
	UGaiaInventoryTestHelper::TestContainerDeletion(this);
}

void AGaiaInventoryTestActor::RunPerformanceTest()
{
	UE_LOG(LogGaia, Log, TEXT("手动运行性能测试 (物品数量: %d)..."), PerformanceTestItemCount);
	UGaiaInventoryTestHelper::TestPerformance(this, PerformanceTestItemCount);
}

