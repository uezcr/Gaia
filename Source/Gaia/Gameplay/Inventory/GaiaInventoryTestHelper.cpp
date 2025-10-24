#include "GaiaInventoryTestHelper.h"
#include "GaiaInventorySubsystem.h"
#include "GaiaInventoryTypes.h"
#include "GaiaLogChannels.h"

void UGaiaInventoryTestHelper::LogTestResult(const FString& TestName, bool bPassed, const FString& Details)
{
	if (bPassed)
	{
		UE_LOG(LogGaia, Log, TEXT("[✓] %s 通过 %s"), *TestName, *Details);
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[✗] %s 失败 %s"), *TestName, *Details);
	}
}

FString UGaiaInventoryTestHelper::FormatMoveResult(const FMoveItemResult& Result)
{
	FString ResultStr = FString::Printf(TEXT("结果类型: %s"), 
		Result.Result == EMoveItemResult::Success ? TEXT("成功") :
		Result.Result == EMoveItemResult::PartialSuccess ? TEXT("部分成功") :
		Result.Result == EMoveItemResult::SwapPerformed ? TEXT("交换完成") :
		Result.Result == EMoveItemResult::Failed ? TEXT("失败") :
		Result.Result == EMoveItemResult::InvalidTarget ? TEXT("无效目标") :
		Result.Result == EMoveItemResult::TypeMismatch ? TEXT("类型不匹配") :
		Result.Result == EMoveItemResult::StackLimitReached ? TEXT("堆叠限制") :
		Result.Result == EMoveItemResult::ContainerRejected ? TEXT("容器拒绝") :
		TEXT("未知"));
	
	if (!Result.ErrorMessage.IsEmpty())
	{
		ResultStr += FString::Printf(TEXT(", 错误信息: %s"), *Result.ErrorMessage);
	}
	
	ResultStr += FString::Printf(TEXT(", 移动数量: %d, 剩余数量: %d"), 
		Result.MovedQuantity, Result.RemainingQuantity);
	
	if (Result.bMovedToContainer)
	{
		ResultStr += TEXT(", 已移动到容器");
	}
	
	if (Result.bWasSwapped)
	{
		ResultStr += FString::Printf(TEXT(", 已交换物品: %s"), *Result.SwappedItemUID.ToString());
	}
	
	return ResultStr;
}

void UGaiaInventoryTestHelper::PrintContainerInfo(UGaiaInventorySubsystem* InventorySystem, const FGuid& ContainerUID, const FString& ContainerName)
{
	FGaiaContainerInstance Container;
	if (!InventorySystem->FindContainerByUID(ContainerUID, Container))
	{
		UE_LOG(LogGaia, Log, TEXT("容器信息: %s - 容器不存在"), *ContainerName);
		return;
	}
	
	// 使用新架构：从AllItems获取容器中的物品
	TArray<FGaiaItemInstance> ItemsInContainer = InventorySystem->GetItemsInContainer(ContainerUID);
	
	FString ContainerInfo = FString::Printf(TEXT("容器信息: %s (UID: %s)"), *ContainerName, *ContainerUID.ToString());
	ContainerInfo += FString::Printf(TEXT(" - 槽位数: %d"), Container.Slots.Num());
	ContainerInfo += FString::Printf(TEXT(", 物品数: %d"), ItemsInContainer.Num());
	
	// 统计每个槽位的物品
	int32 UsedSlots = 0;
	for (int32 i = 0; i < Container.Slots.Num(); i++)
	{
		if (!Container.Slots[i].IsEmpty())
		{
			UsedSlots++;
			FGaiaItemInstance Item;
			if (InventorySystem->FindItemByUID(Container.Slots[i].ItemInstanceUID, Item))
			{
				ContainerInfo += FString::Printf(TEXT(", 槽位%d: %s(x%d)"), i, *Item.ItemDefinitionID.ToString(), Item.Quantity);
			}
		}
	}
	
	ContainerInfo += FString::Printf(TEXT(", 已使用槽位: %d"), UsedSlots);
	UE_LOG(LogGaia, Log, TEXT("%s"), *ContainerInfo);
}

bool UGaiaInventoryTestHelper::RunBasicTests(const UObject* WorldContextObject)
{
	UE_LOG(LogGaia, Warning, TEXT("========================================"));
	UE_LOG(LogGaia, Warning, TEXT("开始运行库存系统基础测试（新架构）"));
	UE_LOG(LogGaia, Warning, TEXT("========================================"));

	// 测试1: 容器创建和查找
	if (!TestContainerCreation(WorldContextObject))
	{
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		UE_LOG(LogGaia, Error, TEXT("测试中止！容器创建测试失败 ✗"));
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		return false;
	}

	// 测试2: 物品创建和UID唯一性
	if (!TestItemCreation(WorldContextObject))
	{
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		UE_LOG(LogGaia, Error, TEXT("测试中止！物品创建测试失败 ✗"));
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		return false;
	}

	// 测试3: 物品添加和查找
	if (!TestAddAndFindItem(WorldContextObject))
	{
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		UE_LOG(LogGaia, Error, TEXT("测试中止！物品添加和查找测试失败 ✗"));
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		return false;
	}

	// 测试4: 物品移除和游离状态
	if (!TestRemoveAndOrphanItem(WorldContextObject))
	{
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		UE_LOG(LogGaia, Error, TEXT("测试中止！物品移除和游离状态测试失败 ✗"));
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		return false;
	}

	// 测试5: 数据一致性验证
	if (!TestDataIntegrity(WorldContextObject))
	{
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		UE_LOG(LogGaia, Error, TEXT("测试中止！数据一致性测试失败 ✗"));
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		return false;
	}

	// 测试6: 容器删除逻辑
	if (!TestContainerDeletion(WorldContextObject))
	{
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		UE_LOG(LogGaia, Error, TEXT("测试中止！容器删除逻辑测试失败 ✗"));
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		return false;
	}

	// 测试7: 物品移动功能
	if (!RunMoveItemTests(WorldContextObject))
	{
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		UE_LOG(LogGaia, Error, TEXT("测试中止！物品移动测试失败 ✗"));
		UE_LOG(LogGaia, Error, TEXT("========================================"));
		return false;
	}

	// 所有测试通过
	UE_LOG(LogGaia, Warning, TEXT("========================================"));
	UE_LOG(LogGaia, Warning, TEXT("所有测试通过！✓"));
	UE_LOG(LogGaia, Warning, TEXT("========================================"));

	return true;
}

bool UGaiaInventoryTestHelper::TestContainerCreation(const UObject* WorldContextObject)
{
	UE_LOG(LogGaia, Log, TEXT(">>> 测试: 容器创建和查找"));

	UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContextObject);
	if (!InventorySystem)
	{
		LogTestResult(TEXT("容器创建"), false, TEXT("- 无法获取库存子系统"));
		return false;
	}

	// 测试1: 创建容器
	FGuid ContainerUID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	if (!ContainerUID.IsValid())
	{
		LogTestResult(TEXT("容器创建"), false, TEXT("- UID无效"));
		return false;
	}

	// 测试2: 查找容器
	FGaiaContainerInstance Container;
	if (!InventorySystem->FindContainerByUID(ContainerUID, Container))
	{
		LogTestResult(TEXT("容器创建"), false, TEXT("- 无法查找容器"));
		return false;
	}

	// 测试3: UID一致性
	if (Container.ContainerUID != ContainerUID)
	{
		LogTestResult(TEXT("容器创建"), false, TEXT("- UID不匹配"));
		return false;
	}

	// 测试4: UID唯一性
	FGuid Container2UID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	FGuid Container3UID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	if (Container2UID == Container3UID || Container2UID == ContainerUID)
	{
		LogTestResult(TEXT("容器创建"), false, TEXT("- UID不唯一"));
		return false;
	}

	LogTestResult(TEXT("容器创建"), true, FString::Printf(TEXT("- 创建了3个容器，UID全部唯一")));
	return true;
}

bool UGaiaInventoryTestHelper::TestItemCreation(const UObject* WorldContextObject)
{
	UE_LOG(LogGaia, Log, TEXT(">>> 测试: 物品创建和UID唯一性"));

	UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContextObject);
	if (!InventorySystem)
	{
		LogTestResult(TEXT("物品创建"), false, TEXT("- 无法获取库存子系统"));
		return false;
	}

	// 测试1: 创建物品
	FGaiaItemInstance Item1 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 10);
	if (!Item1.IsValid())
	{
		LogTestResult(TEXT("物品创建"), false, TEXT("- 物品无效"));
		return false;
	}

	// 测试2: 数量验证
	if (Item1.Quantity != 10)
	{
		LogTestResult(TEXT("物品创建"), false, FString::Printf(TEXT("- 数量错误: 期望10, 实际%d"), Item1.Quantity));
		return false;
	}

	// 测试3: UID唯一性
	FGaiaItemInstance Item2 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 5);
	FGaiaItemInstance Item3 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 15);

	if (Item1.InstanceUID == Item2.InstanceUID || Item1.InstanceUID == Item3.InstanceUID || Item2.InstanceUID == Item3.InstanceUID)
	{
		LogTestResult(TEXT("物品创建"), false, TEXT("- UID不唯一"));
		return false;
	}

	LogTestResult(TEXT("物品创建"), true, FString::Printf(TEXT("- 创建了3个物品，数量正确，UID全部唯一")));
	return true;
}

bool UGaiaInventoryTestHelper::TestAddAndFindItem(const UObject* WorldContextObject)
{
	UE_LOG(LogGaia, Log, TEXT(">>> 测试: 物品添加和查找"));

	UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContextObject);
	if (!InventorySystem)
	{
		LogTestResult(TEXT("物品添加和查找"), false, TEXT("- 无法获取库存子系统"));
		return false;
	}

	// 创建容器和物品
	FGuid ContainerUID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	FGaiaItemInstance Item = InventorySystem->CreateItemInstance(TEXT("TestItem"), 5);

	if (!ContainerUID.IsValid() || !Item.IsValid())
	{
		LogTestResult(TEXT("物品添加和查找"), false, TEXT("- 创建容器或物品失败"));
		return false;
	}

	// 测试1: 添加物品
	bool bAddSuccess = InventorySystem->AddItemToContainer(Item, ContainerUID);
	if (!bAddSuccess)
	{
		LogTestResult(TEXT("物品添加和查找"), false, TEXT("- 添加物品失败"));
		return false;
	}

	// 测试2: 验证容器中的物品数量
	TArray<FGaiaItemInstance> ItemsInContainer = InventorySystem->GetItemsInContainer(ContainerUID);
	if (ItemsInContainer.Num() != 1)
	{
		LogTestResult(TEXT("物品添加和查找"), false, FString::Printf(TEXT("- 容器物品数量错误: %d"), ItemsInContainer.Num()));
		return false;
	}

	// 测试3: 通过UID查找物品（O(1)查找）
	FGaiaItemInstance FoundItem;
	if (!InventorySystem->FindItemByUID(Item.InstanceUID, FoundItem))
	{
		LogTestResult(TEXT("物品添加和查找"), false, TEXT("- 通过UID查找物品失败"));
		return false;
	}

	// 测试4: 验证查找到的物品
	if (FoundItem.InstanceUID != Item.InstanceUID || FoundItem.Quantity != Item.Quantity)
	{
		LogTestResult(TEXT("物品添加和查找"), false, TEXT("- 查找到的物品数据不匹配"));
		return false;
	}

	LogTestResult(TEXT("物品添加和查找"), true, TEXT("- 添加和O(1)查找功能正常"));
	return true;
}

bool UGaiaInventoryTestHelper::TestRemoveItem(const UObject* WorldContextObject)
{
	UE_LOG(LogGaia, Log, TEXT(">>> 测试: 物品删除和索引重建"));

	UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContextObject);
	if (!InventorySystem)
	{
		LogTestResult(TEXT("物品删除"), false, TEXT("- 无法获取库存子系统"));
		return false;
	}

	// 创建容器并添加多个物品
	FGuid ContainerUID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	FGaiaItemInstance Item1 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 5);
	FGaiaItemInstance Item2 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 10);
	FGaiaItemInstance Item3 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 15);

	InventorySystem->AddItemToContainer(Item1, ContainerUID);
	InventorySystem->AddItemToContainer(Item2, ContainerUID);
	InventorySystem->AddItemToContainer(Item3, ContainerUID);

	TArray<FGaiaItemInstance> ItemsInContainer = InventorySystem->GetItemsInContainer(ContainerUID);
	if (ItemsInContainer.Num() != 3)
	{
		LogTestResult(TEXT("物品删除"), false, TEXT("- 容器初始化失败"));
		return false;
	}

	// 测试1: 移除中间的物品（设为游离状态）
	bool bRemoveSuccess = InventorySystem->RemoveItemFromContainer(Item2.InstanceUID);
	if (!bRemoveSuccess)
	{
		LogTestResult(TEXT("物品删除"), false, TEXT("- 移除物品失败"));
		return false;
	}

	// 测试2: 验证容器物品数量
	ItemsInContainer = InventorySystem->GetItemsInContainer(ContainerUID);
	if (ItemsInContainer.Num() != 2)
	{
		LogTestResult(TEXT("物品删除"), false, FString::Printf(TEXT("- 移除后数量错误: %d"), ItemsInContainer.Num()));
		return false;
	}

	// 测试3: 验证被移除的物品变为游离状态
	FGaiaItemInstance RemovedItem;
	if (!InventorySystem->FindItemByUID(Item2.InstanceUID, RemovedItem))
	{
		LogTestResult(TEXT("物品删除"), false, TEXT("- 被移除的物品找不到（应该是游离状态）"));
		return false;
	}
	
	if (!RemovedItem.IsOrphan())
	{
		LogTestResult(TEXT("物品删除"), false, TEXT("- 被移除的物品不是游离状态"));
		return false;
	}

	// 测试4: 验证剩余物品可以查找
	FGaiaItemInstance Item1Found;
	FGaiaItemInstance Item3Found;

	if (!InventorySystem->FindItemByUID(Item1.InstanceUID, Item1Found) || !InventorySystem->FindItemByUID(Item3.InstanceUID, Item3Found))
	{
		LogTestResult(TEXT("物品删除"), false, TEXT("- 索引重建后查找失败"));
		return false;
	}

	LogTestResult(TEXT("物品删除"), true, TEXT("- 删除功能和索引重建正常"));
	return true;
}

bool UGaiaInventoryTestHelper::TestPerformance(const UObject* WorldContextObject, int32 ItemCount)
{
	UE_LOG(LogGaia, Log, TEXT(">>> 测试: 性能测试 (物品数量: %d)"), ItemCount);

	UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContextObject);
	if (!InventorySystem)
	{
		LogTestResult(TEXT("性能测试"), false, TEXT("- 无法获取库存子系统"));
		return false;
	}

	// 创建容器
	FGuid ContainerUID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	
	TArray<FGuid> ItemUIDs;
	ItemUIDs.Reserve(ItemCount);

	// 测试1: 批量创建和添加物品
	double StartTime = FPlatformTime::Seconds();
	
	for (int32 i = 0; i < ItemCount; i++)
	{
		FGaiaItemInstance Item = InventorySystem->CreateItemInstance(TEXT("TestItem"), i + 1);
		ItemUIDs.Add(Item.InstanceUID);
		InventorySystem->AddItemToContainer(Item, ContainerUID);
	}
	
	double CreateTime = FPlatformTime::Seconds() - StartTime;

	// 测试2: 批量查找性能（O(1)查找）
	StartTime = FPlatformTime::Seconds();
	
	int32 FoundCount = 0;
	FGaiaItemInstance TempItem;
	for (const FGuid& ItemUID : ItemUIDs)
	{
		if (InventorySystem->FindItemByUID(ItemUID, TempItem))
		{
			FoundCount++;
		}
	}
	
	double LookupTime = FPlatformTime::Seconds() - StartTime;

	// 验证
	if (FoundCount != ItemCount)
	{
		LogTestResult(TEXT("性能测试"), false, FString::Printf(TEXT("- 查找数量不匹配: %d/%d"), FoundCount, ItemCount));
		return false;
	}

	// 计算性能指标
	double AvgCreateTime = (CreateTime / ItemCount) * 1000000.0; // 微秒
	double AvgLookupTime = (LookupTime / ItemCount) * 1000000.0; // 微秒

	FString PerformanceDetails = FString::Printf(
		TEXT("- 创建: %.2fms (%.2fμs/个), 查找: %.2fms (%.2fμs/个)"),
		CreateTime * 1000.0, AvgCreateTime,
		LookupTime * 1000.0, AvgLookupTime
	);

	// O(1)查找应该非常快，平均小于10微秒
	bool bPerformanceGood = AvgLookupTime < 10.0;
	
	if (bPerformanceGood)
	{
		LogTestResult(TEXT("性能测试"), true, PerformanceDetails);
	}
	else
	{
		LogTestResult(TEXT("性能测试"), false, PerformanceDetails + TEXT(" (查找性能不佳)"));
	}

	return bPerformanceGood;
}

bool UGaiaInventoryTestHelper::RunMoveItemTests(const UObject* WorldContextObject)
{
	UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContextObject);
	if (!InventorySystem)
	{
		LogTestResult(TEXT("移动物品测试"), false, TEXT("- 无法获取库存系统"));
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("=== 开始移动物品测试 ==="));

	// 创建测试容器
	FGuid Container1UID = InventorySystem->CreateContainer(TEXT("TestContainer"));
	FGuid Container2UID = InventorySystem->CreateContainer(TEXT("TestContainer"));
	
	if (!Container1UID.IsValid() || !Container2UID.IsValid())
	{
		LogTestResult(TEXT("移动物品测试"), false, TEXT("- 无法创建测试容器"));
		return false;
	}

	// 测试1: 移动到空槽位
	FGaiaItemInstance Item1 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 10);
	InventorySystem->AddItemToContainer(Item1, Container1UID);
	
	FMoveItemResult MoveResult = InventorySystem->MoveItem(Item1.InstanceUID, Container2UID, 0, 5);
	if (!MoveResult.IsSuccess())
	{
		LogTestResult(TEXT("移动到空槽位"), false, FString::Printf(TEXT("- 移动失败: %s"), *FormatMoveResult(MoveResult)));
		return false;
	}
	
	LogTestResult(TEXT("移动到空槽位"), true, FString::Printf(TEXT("- 成功移动 %d 个物品"), MoveResult.MovedQuantity));
	
	// 打印容器最终状态
	PrintContainerInfo(InventorySystem, Container1UID, TEXT("Container1"));
	PrintContainerInfo(InventorySystem, Container2UID, TEXT("Container2"));

	// 测试2: 堆叠相同物品
	FGaiaItemInstance Item2 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 3);
	InventorySystem->AddItemToContainer(Item2, Container2UID);
	
	FMoveItemResult StackResult = InventorySystem->MoveItem(Item2.InstanceUID, Container2UID, 0, 3);
	if (!StackResult.IsSuccess())
	{
		LogTestResult(TEXT("堆叠相同物品"), false, FString::Printf(TEXT("- 堆叠失败: %s"), *FormatMoveResult(StackResult)));
		return false;
	}
	
	LogTestResult(TEXT("堆叠相同物品"), true, FString::Printf(TEXT("- 成功堆叠 %d 个物品"), StackResult.MovedQuantity));
	
	// 打印容器最终状态
	PrintContainerInfo(InventorySystem, Container1UID, TEXT("Container1"));
	PrintContainerInfo(InventorySystem, Container2UID, TEXT("Container2"));

	// 测试3: 部分堆叠
	FGaiaItemInstance Item3 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 20);
	InventorySystem->AddItemToContainer(Item3, Container1UID);
	
	FMoveItemResult PartialStackResult = InventorySystem->MoveItem(Item3.InstanceUID, Container2UID, 0, 20);
	if (PartialStackResult.Result != EMoveItemResult::PartialSuccess)
	{
		LogTestResult(TEXT("部分堆叠"), false, FString::Printf(TEXT("- 部分堆叠失败: %s"), *FormatMoveResult(PartialStackResult)));
		return false;
	}
	
	LogTestResult(TEXT("部分堆叠"), true, FString::Printf(TEXT("- 部分堆叠成功: 移动=%d, 剩余=%d"), PartialStackResult.MovedQuantity, PartialStackResult.RemainingQuantity));
	
	// 打印容器最终状态
	PrintContainerInfo(InventorySystem, Container1UID, TEXT("Container1"));
	PrintContainerInfo(InventorySystem, Container2UID, TEXT("Container2"));

	// 测试4: 交换不同物品
	FGaiaItemInstance Item4 = InventorySystem->CreateItemInstance(TEXT("TestItem2"), 5);
	InventorySystem->AddItemToContainer(Item4, Container1UID);
	
	FMoveItemResult SwapResult = InventorySystem->MoveItem(Item4.InstanceUID, Container2UID, 0, 5);
	if (SwapResult.Result != EMoveItemResult::SwapPerformed)
	{
		LogTestResult(TEXT("交换不同物品"), false, FString::Printf(TEXT("- 交换失败: %s"), *FormatMoveResult(SwapResult)));
		return false;
	}
	
	LogTestResult(TEXT("交换不同物品"), true, FString::Printf(TEXT("- 成功交换物品")));
	
	// 打印容器最终状态
	PrintContainerInfo(InventorySystem, Container1UID, TEXT("Container1"));
	PrintContainerInfo(InventorySystem, Container2UID, TEXT("Container2"));

	// 测试5: 快速移动
	FGaiaItemInstance Item5 = InventorySystem->CreateItemInstance(TEXT("TestItem3"), 7);
	InventorySystem->AddItemToContainer(Item5, Container1UID);
	
	FMoveItemResult QuickMoveResult = InventorySystem->QuickMoveItem(Item5.InstanceUID, Container2UID);
	if (!QuickMoveResult.IsSuccess())
	{
		LogTestResult(TEXT("快速移动"), false, FString::Printf(TEXT("- 快速移动失败: %s"), *FormatMoveResult(QuickMoveResult)));
		return false;
	}
	
	LogTestResult(TEXT("快速移动"), true, FString::Printf(TEXT("- 成功快速移动 %d 个物品"), QuickMoveResult.MovedQuantity));
	
	// 打印容器最终状态
	PrintContainerInfo(InventorySystem, Container1UID, TEXT("Container1"));
	PrintContainerInfo(InventorySystem, Container2UID, TEXT("Container2"));

	// 测试6: 拆分移动
	FGaiaItemInstance Item6 = InventorySystem->CreateItemInstance(TEXT("TestItem4"), 15);
	InventorySystem->AddItemToContainer(Item6, Container1UID);
	
	FMoveItemResult SplitResult = InventorySystem->SplitItem(Item6.InstanceUID, Container2UID, 8);
	if (!SplitResult.IsSuccess())
	{
		LogTestResult(TEXT("拆分移动"), false, FString::Printf(TEXT("- 拆分移动失败: %s"), *FormatMoveResult(SplitResult)));
		return false;
	}
	
	if (SplitResult.MovedQuantity != 8)
	{
		LogTestResult(TEXT("拆分移动"), false, FString::Printf(TEXT("- 拆分数量错误: %d"), SplitResult.MovedQuantity));
		return false;
	}
	
	LogTestResult(TEXT("拆分移动"), true, FString::Printf(TEXT("- 成功拆分移动 %d 个物品"), SplitResult.MovedQuantity));
	
	// 打印容器最终状态
	PrintContainerInfo(InventorySystem, Container1UID, TEXT("Container1"));
	PrintContainerInfo(InventorySystem, Container2UID, TEXT("Container2"));

	// 测试7: 容器内移动
	FMoveItemResult WithinContainerResult = InventorySystem->MoveItem(Item6.InstanceUID, Container1UID, 2, 3);
	if (!WithinContainerResult.IsSuccess())
	{
		LogTestResult(TEXT("容器内移动"), false, FString::Printf(TEXT("- 容器内移动失败: %s"), *FormatMoveResult(WithinContainerResult)));
		return false;
	}
	
	LogTestResult(TEXT("容器内移动"), true, FString::Printf(TEXT("- 成功容器内移动 %d 个物品"), WithinContainerResult.MovedQuantity));
	
	// 打印容器最终状态
	PrintContainerInfo(InventorySystem, Container1UID, TEXT("Container1"));
	PrintContainerInfo(InventorySystem, Container2UID, TEXT("Container2"));

	// 测试8: 自动分配槽位
	FGaiaItemInstance Item7 = InventorySystem->CreateItemInstance(TEXT("TestItem5"), 4);
	InventorySystem->AddItemToContainer(Item7, Container1UID);
	
	FMoveItemResult AutoSlotResult = InventorySystem->MoveItem(Item7.InstanceUID, Container2UID, -1, 4);
	if (!AutoSlotResult.IsSuccess())
	{
		LogTestResult(TEXT("自动分配槽位"), false, FString::Printf(TEXT("- 自动分配槽位失败: %s"), *FormatMoveResult(AutoSlotResult)));
		return false;
	}
	
	LogTestResult(TEXT("自动分配槽位"), true, FString::Printf(TEXT("- 成功自动分配槽位移动 %d 个物品"), AutoSlotResult.MovedQuantity));
	
	// 打印容器最终状态
	PrintContainerInfo(InventorySystem, Container1UID, TEXT("Container1"));
	PrintContainerInfo(InventorySystem, Container2UID, TEXT("Container2"));

	UE_LOG(LogGaia, Log, TEXT("=== 移动物品测试完成 ==="));
	return true;
}

bool UGaiaInventoryTestHelper::TestRemoveAndOrphanItem(const UObject* WorldContextObject)
{
	UE_LOG(LogGaia, Log, TEXT(">>> 测试: 物品移除和游离状态（新架构）"));

	UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContextObject);
	if (!InventorySystem)
	{
		LogTestResult(TEXT("物品移除"), false, TEXT("- 无法获取库存子系统"));
		return false;
	}

	// 创建容器并添加物品
	FGuid ContainerUID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	FGaiaItemInstance Item1 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 10);
	FGaiaItemInstance Item2 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 20);
	
	InventorySystem->AddItemToContainer(Item1, ContainerUID);
	InventorySystem->AddItemToContainer(Item2, ContainerUID);

	// 测试1: 移除物品（应该变为游离状态，不删除）
	bool bRemoveSuccess = InventorySystem->RemoveItemFromContainer(Item1.InstanceUID);
	if (!bRemoveSuccess)
	{
		LogTestResult(TEXT("移除物品到游离状态"), false, TEXT("- 移除失败"));
		return false;
	}

	// 测试2: 验证物品仍存在于AllItems中
	FGaiaItemInstance RemovedItem;
	if (!InventorySystem->FindItemByUID(Item1.InstanceUID, RemovedItem))
	{
		LogTestResult(TEXT("移除物品到游离状态"), false, TEXT("- 物品被错误删除"));
		return false;
	}

	// 测试3: 验证物品处于游离状态
	if (RemovedItem.IsInContainer())
	{
		LogTestResult(TEXT("移除物品到游离状态"), false, TEXT("- 物品仍在容器中"));
		return false;
	}

	if (!RemovedItem.IsOrphan())
	{
		LogTestResult(TEXT("移除物品到游离状态"), false, TEXT("- 物品不是游离状态"));
		return false;
	}

	LogTestResult(TEXT("移除物品到游离状态"), true, TEXT("- 物品正确设为游离状态"));

	// 测试4: 获取所有游离物品
	TArray<FGaiaItemInstance> OrphanItems = InventorySystem->GetOrphanItems();
	if (OrphanItems.Num() != 1)
	{
		LogTestResult(TEXT("获取游离物品"), false, FString::Printf(TEXT("- 游离物品数量错误: %d"), OrphanItems.Num()));
		return false;
	}

	LogTestResult(TEXT("获取游离物品"), true, TEXT("- 成功获取游离物品列表"));

	// 测试5: 完全删除物品
	bool bDestroySuccess = InventorySystem->DestroyItem(Item1.InstanceUID);
	if (!bDestroySuccess)
	{
		LogTestResult(TEXT("完全删除物品"), false, TEXT("- 删除失败"));
		return false;
	}

	// 测试6: 验证物品已从AllItems中删除
	FGaiaItemInstance DestroyedItem;
	if (InventorySystem->FindItemByUID(Item1.InstanceUID, DestroyedItem))
	{
		LogTestResult(TEXT("完全删除物品"), false, TEXT("- 物品未被删除"));
		return false;
	}

	LogTestResult(TEXT("完全删除物品"), true, TEXT("- 物品成功从系统中删除"));

	// 测试7: 删除容器中的物品（应该先移除再删除）
	bool bDestroyInContainerSuccess = InventorySystem->DestroyItem(Item2.InstanceUID);
	if (!bDestroyInContainerSuccess)
	{
		LogTestResult(TEXT("删除容器中的物品"), false, TEXT("- 删除失败"));
		return false;
	}

	// 测试8: 验证容器槽位已清空
	FGaiaContainerInstance Container;
	if (!InventorySystem->FindContainerByUID(ContainerUID, Container))
	{
		LogTestResult(TEXT("删除容器中的物品"), false, TEXT("- 无法找到容器"));
		return false;
	}

	TArray<FGaiaItemInstance> ItemsInContainer = InventorySystem->GetItemsInContainer(ContainerUID);
	if (ItemsInContainer.Num() != 0)
	{
		LogTestResult(TEXT("删除容器中的物品"), false, FString::Printf(TEXT("- 容器中仍有物品: %d"), ItemsInContainer.Num()));
		return false;
	}

	LogTestResult(TEXT("删除容器中的物品"), true, TEXT("- 成功删除容器中的物品"));

	UE_LOG(LogGaia, Log, TEXT("=== 物品移除和游离状态测试完成 ==="));
	return true;
}

bool UGaiaInventoryTestHelper::TestDataIntegrity(const UObject* WorldContextObject)
{
	UE_LOG(LogGaia, Log, TEXT(">>> 测试: 数据一致性验证（新架构）"));

	UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContextObject);
	if (!InventorySystem)
	{
		LogTestResult(TEXT("数据一致性"), false, TEXT("- 无法获取库存子系统"));
		return false;
	}

	// 创建测试数据
	FGuid Container1UID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	FGuid Container2UID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	
	FGaiaItemInstance Item1 = InventorySystem->CreateItemInstance(TEXT("TestDataIntegrity"), 10);
	FGaiaItemInstance Item2 = InventorySystem->CreateItemInstance(TEXT("TestDataIntegrity"), 20);
	FGaiaItemInstance Item3 = InventorySystem->CreateItemInstance(TEXT("TestDataIntegrity"), 30);
	
	InventorySystem->AddItemToContainer(Item1, Container1UID);
	InventorySystem->AddItemToContainer(Item2, Container1UID);
	InventorySystem->AddItemToContainer(Item3, Container2UID);

	// 测试1: 验证数据一致性
	bool bIsValid = InventorySystem->ValidateDataIntegrity();
	if (!bIsValid)
	{
		LogTestResult(TEXT("数据一致性验证"), false, TEXT("- 数据一致性验证失败"));
		return false;
	}

	LogTestResult(TEXT("数据一致性验证"), true, TEXT("- 数据一致性验证通过"));

	// 测试2: 测试GetItemsInContainer
	TArray<FGaiaItemInstance> Container1Items = InventorySystem->GetItemsInContainer(Container1UID);
	if (Container1Items.Num() != 2)
	{
		LogTestResult(TEXT("获取容器物品"), false, FString::Printf(TEXT("- 容器1物品数量错误: %d"), Container1Items.Num()));
		return false;
	}

	LogTestResult(TEXT("获取容器物品"), true, FString::Printf(TEXT("- 成功获取容器物品: %d 个"), Container1Items.Num()));

	// 测试3: 测试CountItemsByType
	int32 TotalCount = InventorySystem->CountItemsByType(TEXT("TestDataIntegrity"));
	if (TotalCount != 60) // 10 + 20 + 30
	{
		LogTestResult(TEXT("统计全局物品数量"), false, FString::Printf(TEXT("- 数量错误: %d (应为60)"), TotalCount));
		return false;
	}

	LogTestResult(TEXT("统计全局物品数量"), true, FString::Printf(TEXT("- 成功统计全局物品: %d 个"), TotalCount));

	UE_LOG(LogGaia, Log, TEXT("=== 数据一致性测试完成 ==="));
	return true;
}

bool UGaiaInventoryTestHelper::TestContainerDeletion(const UObject* WorldContextObject)
{
	UE_LOG(LogGaia, Log, TEXT(">>> 测试: 容器删除逻辑（新架构）"));

	UGaiaInventorySubsystem* InventorySystem = UGaiaInventorySubsystem::Get(WorldContextObject);
	if (!InventorySystem)
	{
		LogTestResult(TEXT("容器删除"), false, TEXT("- 无法获取库存子系统"));
		return false;
	}

	// 创建主背包
	FGuid MainBackpackUID = InventorySystem->CreateContainer(TEXT("TestBackpackBig"));
	
	// 创建一个有容器的物品（子背包）
	FGaiaItemInstance SubBackpack = InventorySystem->CreateItemInstance(TEXT("TestBackpackSmall"), 1);
	if (!SubBackpack.HasContainer())
	{
		LogTestResult(TEXT("创建子背包"), false, TEXT("- 背包物品没有容器"));
		return false;
	}
	
	FGuid SubBackpackContainerUID = SubBackpack.OwnedContainerUID;
	
	// 在子背包中放入物品
	FGaiaItemInstance Item1 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 10);
	FGaiaItemInstance Item2 = InventorySystem->CreateItemInstance(TEXT("TestItem"), 20);
	
	InventorySystem->AddItemToContainer(Item1, SubBackpackContainerUID);
	InventorySystem->AddItemToContainer(Item2, SubBackpackContainerUID);
	
	// 将子背包放入主背包
	InventorySystem->AddItemToContainer(SubBackpack, MainBackpackUID);
	
	// 测试1: 验证容器关系
	FGaiaContainerInstance SubContainer;
	if (!InventorySystem->FindContainerByUID(SubBackpackContainerUID, SubContainer))
	{
		LogTestResult(TEXT("验证容器关系"), false, TEXT("- 无法找到子容器"));
		return false;
	}
	
	if (SubContainer.ParentContainerUID != MainBackpackUID)
	{
		LogTestResult(TEXT("验证容器关系"), false, TEXT("- 子容器父容器引用错误"));
		return false;
	}
	
	LogTestResult(TEXT("验证容器关系"), true, TEXT("- 容器嵌套关系正确"));
	
	// 测试2: 删除子背包，验证其容器和内容物都被删除
	bool bDestroySuccess = InventorySystem->DestroyItem(SubBackpack.InstanceUID);
	if (!bDestroySuccess)
	{
		LogTestResult(TEXT("删除背包"), false, TEXT("- 删除失败"));
		return false;
	}
	
	// 测试3: 验证子背包物品已被删除
	FGaiaItemInstance CheckSubBackpack;
	if (InventorySystem->FindItemByUID(SubBackpack.InstanceUID, CheckSubBackpack))
	{
		LogTestResult(TEXT("验证背包删除"), false, TEXT("- 背包物品未被删除"));
		return false;
	}
	
	LogTestResult(TEXT("验证背包删除"), true, TEXT("- 背包物品已删除"));
	
	// 测试4: 验证子容器已被删除
	FGaiaContainerInstance CheckSubContainer;
	if (InventorySystem->FindContainerByUID(SubBackpackContainerUID, CheckSubContainer))
	{
		LogTestResult(TEXT("验证容器删除"), false, TEXT("- 容器未被删除"));
		return false;
	}
	
	LogTestResult(TEXT("验证容器删除"), true, TEXT("- 容器已删除"));
	
	// 测试5: 验证容器内的物品已被删除
	FGaiaItemInstance CheckItem1;
	FGaiaItemInstance CheckItem2;
	
	if (InventorySystem->FindItemByUID(Item1.InstanceUID, CheckItem1))
	{
		LogTestResult(TEXT("验证容器内物品删除"), false, TEXT("- 容器内物品1未被删除"));
		return false;
	}
	
	if (InventorySystem->FindItemByUID(Item2.InstanceUID, CheckItem2))
	{
		LogTestResult(TEXT("验证容器内物品删除"), false, TEXT("- 容器内物品2未被删除"));
		return false;
	}
	
	LogTestResult(TEXT("验证容器内物品删除"), true, TEXT("- 容器内所有物品已删除"));
	
	// 测试6: 验证主背包槽位已清空
	FGaiaContainerInstance MainContainer;
	if (!InventorySystem->FindContainerByUID(MainBackpackUID, MainContainer))
	{
		LogTestResult(TEXT("验证主背包状态"), false, TEXT("- 无法找到主背包"));
		return false;
	}
	
	TArray<FGaiaItemInstance> MainContainerItems = InventorySystem->GetItemsInContainer(MainBackpackUID);
	if (MainContainerItems.Num() != 0)
	{
		LogTestResult(TEXT("验证主背包状态"), false, FString::Printf(TEXT("- 主背包仍有物品: %d"), MainContainerItems.Num()));
		return false;
	}
	
	LogTestResult(TEXT("验证主背包状态"), true, TEXT("- 主背包已清空"));
	
	UE_LOG(LogGaia, Log, TEXT("=== 容器删除逻辑测试完成 ==="));
	return true;
}

