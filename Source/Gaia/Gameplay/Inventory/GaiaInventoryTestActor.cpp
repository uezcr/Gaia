#include "GaiaInventoryTestActor.h"
#include "GaiaInventoryTestHelper.h"

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
			RunTests();
		});
	}
}

void AGaiaInventoryTestActor::RunTests()
{
	UGaiaInventoryTestHelper::RunBasicTests(this);
}

void AGaiaInventoryTestActor::RunContainerTest()
{
	UGaiaInventoryTestHelper::TestContainerCreation(this);
}

void AGaiaInventoryTestActor::RunItemTest()
{
	UGaiaInventoryTestHelper::TestItemCreation(this);
}

void AGaiaInventoryTestActor::RunAddFindTest()
{
	UGaiaInventoryTestHelper::TestAddAndFindItem(this);
}

void AGaiaInventoryTestActor::RunRemoveTest()
{
	UGaiaInventoryTestHelper::TestRemoveItem(this);
}

void AGaiaInventoryTestActor::RunPerformanceTest()
{
	UGaiaInventoryTestHelper::TestPerformance(this, PerformanceTestItemCount);
}

