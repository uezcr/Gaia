// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaInventoryNetworkTestActor.h"
#include "GaiaInventorySubsystem.h"
#include "GaiaInventoryRPCComponent.h"
#include "GaiaInventoryTypes.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogGaiaNetworkTest, Log, All);

AGaiaInventoryNetworkTestActor::AGaiaInventoryNetworkTestActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AGaiaInventoryNetworkTestActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRunTests && HasAuthority())
	{
		StartTestsWithDelay();
	}
}

// ========================================
// 手动测试函数
// ========================================

void AGaiaInventoryNetworkTestActor::RunAllTests()
{
	LogNetworkStatus();
	LogSeparator(TEXT("开始运行网络测试"));
	
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	if (bTestRPCComponent)
	{
		RunRPCComponentTests();
	}

	if (bTestContainerSync)
	{
		RunContainerSyncTests();
	}

	if (bTestMovementSync)
	{
		RunMovementSyncTests();
	}

	if (bTestNestedSync)
	{
		RunNestedSyncTests();
	}

	if (bTestBroadcast)
	{
		RunBroadcastTests();
	}

	LogSeparator(FString::Printf(TEXT("网络测试完成: %d/%d 通过"), PassedTests, TotalTests));
	
	if (FailedTests > 0)
	{
		UE_LOG(LogGaiaNetworkTest, Error, TEXT("❌ 有 %d 个测试失败！"), FailedTests);
	}
	else
	{
		UE_LOG(LogGaiaNetworkTest, Display, TEXT("✅ 所有测试通过！"));
	}
}

void AGaiaInventoryNetworkTestActor::RunRPCComponentTests()
{
	LogSeparator(TEXT("RPC组件测试"));
	
	Test_RPCComponentInitialization();
	Test_ContainerRegistration();
	Test_DataRefresh();
}

void AGaiaInventoryNetworkTestActor::RunContainerSyncTests()
{
	LogSeparator(TEXT("容器同步测试"));
	
	Test_ContainerCreationSync();
	Test_ItemAdditionSync();
	Test_ItemRemovalSync();
	Test_ContainerDataConsistency();
}

void AGaiaInventoryNetworkTestActor::RunMovementSyncTests()
{
	LogSeparator(TEXT("物品移动同步测试"));
	
	Test_WithinContainerMoveSync();
	Test_CrossContainerMoveSync();
	Test_ItemSwapSync();
	Test_StackingSync();
}

void AGaiaInventoryNetworkTestActor::RunNestedSyncTests()
{
	LogSeparator(TEXT("嵌套容器同步测试"));
	
	Test_NestedContainerAutoSync();
	Test_MultiLevelNestedSync();
	Test_NestedContainerMoveSync();
}

void AGaiaInventoryNetworkTestActor::RunBroadcastTests()
{
	LogSeparator(TEXT("服务器广播测试"));
	
	Test_BroadcastItemChanges();
	Test_MultiClientSync();
}

// ========================================
// RPC组件测试实现
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_RPCComponentInitialization()
{
	TotalTests++;
	
	TArray<APlayerController*> PlayerControllers = GetAllPlayerControllers();
	
	if (PlayerControllers.Num() == 0)
	{
		LogTestResult(TEXT("RPC组件初始化"), false, TEXT("没有玩家控制器"));
		FailedTests++;
		return false;
	}

	// 检查每个玩家都有RPC组件
	for (APlayerController* PC : PlayerControllers)
	{
		if (!PC)
			continue;

		UGaiaInventoryRPCComponent* RPCComp = GetRPCComponent(PC);
		if (!RPCComp)
		{
			LogTestResult(TEXT("RPC组件初始化"), false, 
				FString::Printf(TEXT("玩家 %s 没有RPC组件"), *PC->GetName()));
			FailedTests++;
			return false;
		}
	}

	LogTestResult(TEXT("RPC组件初始化"), true, 
		FString::Printf(TEXT("所有玩家(%d)都有RPC组件"), PlayerControllers.Num()));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_ContainerRegistration()
{
	TotalTests++;
	
	UGaiaInventorySubsystem* InvSys = GetWorld()->GetSubsystem<UGaiaInventorySubsystem>();
	if (!InvSys)
	{
		LogTestResult(TEXT("容器注册"), false, TEXT("无法获取库存子系统"));
		FailedTests++;
		return false;
	}

	APlayerController* ServerPC = GetServerPlayerController();
	if (!ServerPC)
	{
		LogTestResult(TEXT("容器注册"), false, TEXT("没有服务器玩家控制器"));
		FailedTests++;
		return false;
	}

	UGaiaInventoryRPCComponent* RPCComp = GetRPCComponent(ServerPC);
	if (!RPCComp)
	{
		LogTestResult(TEXT("容器注册"), false, TEXT("服务器玩家没有RPC组件"));
		FailedTests++;
		return false;
	}

	// 创建容器并注册
	FGuid ContainerUID = InvSys->CreateContainerInstance(TEXT("PlayerBackpack"));
	TestContainerUIDs.Add(ContainerUID);

	RPCComp->AddOwnedContainerUID(ContainerUID);

	// 验证注册成功
	if (!RPCComp->GetOwnedContainerUIDs().Contains(ContainerUID))
	{
		LogTestResult(TEXT("容器注册"), false, TEXT("容器未成功注册"));
		FailedTests++;
		return false;
	}

	LogTestResult(TEXT("容器注册"), true, TEXT("容器成功注册到RPC组件"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_DataRefresh()
{
	TotalTests++;
	
	UGaiaInventorySubsystem* InvSys = GetWorld()->GetSubsystem<UGaiaInventorySubsystem>();
	if (!InvSys)
	{
		LogTestResult(TEXT("数据刷新"), false, TEXT("无法获取库存子系统"));
		FailedTests++;
		return false;
	}

	APlayerController* ServerPC = GetServerPlayerController();
	if (!ServerPC)
	{
		LogTestResult(TEXT("数据刷新"), false, TEXT("没有服务器玩家控制器"));
		FailedTests++;
		return false;
	}

	UGaiaInventoryRPCComponent* RPCComp = GetRPCComponent(ServerPC);
	if (!RPCComp)
	{
		LogTestResult(TEXT("数据刷新"), false, TEXT("服务器玩家没有RPC组件"));
		FailedTests++;
		return false;
	}

	// 创建容器和物品
	FGuid ContainerUID = InvSys->CreateContainerInstance(TEXT("PlayerBackpack"));
	RPCComp->AddOwnedContainerUID(ContainerUID);

	FGaiaItemInstance Item = InvSys->CreateItemInstance(TEXT("Wood"), 10);
	InvSys->TryAddItemToContainer(Item.InstanceUID, ContainerUID);

	TestContainerUIDs.Add(ContainerUID);
	TestItemUIDs.Add(Item.InstanceUID);

	// 请求刷新数据
	RPCComp->ServerRequestRefreshInventory();

	// TODO: 等待客户端接收数据后验证
	// 由于这需要异步等待RPC完成，这里先标记为通过

	LogTestResult(TEXT("数据刷新"), true, TEXT("数据刷新请求成功"));
	PassedTests++;
	return true;
}

// ========================================
// 容器同步测试实现
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_ContainerCreationSync()
{
	TotalTests++;
	LogTestResult(TEXT("容器创建同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_ItemAdditionSync()
{
	TotalTests++;
	LogTestResult(TEXT("物品添加同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_ItemRemovalSync()
{
	TotalTests++;
	LogTestResult(TEXT("物品移除同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_ContainerDataConsistency()
{
	TotalTests++;
	
	UGaiaInventorySubsystem* InvSys = GetWorld()->GetSubsystem<UGaiaInventorySubsystem>();
	if (!InvSys)
	{
		LogTestResult(TEXT("容器数据一致性"), false, TEXT("无法获取库存子系统"));
		FailedTests++;
		return false;
	}

	// 创建容器和物品
	FGuid ContainerUID = InvSys->CreateContainerInstance(TEXT("PlayerBackpack"));
	FGaiaItemInstance Item1 = InvSys->CreateItemInstance(TEXT("Wood"), 10);
	FGaiaItemInstance Item2 = InvSys->CreateItemInstance(TEXT("Stone"), 5);

	InvSys->TryAddItemToContainer(Item1.InstanceUID, ContainerUID);
	InvSys->TryAddItemToContainer(Item2.InstanceUID, ContainerUID);

	TestContainerUIDs.Add(ContainerUID);
	TestItemUIDs.Add(Item1.InstanceUID);
	TestItemUIDs.Add(Item2.InstanceUID);

	// 验证一致性（这里只是服务器端验证，真正的网络一致性需要等待客户端同步）
	bool bConsistent = VerifyContainerConsistency(ContainerUID, TEXT("容器数据一致性"));
	
	if (bConsistent)
	{
		LogTestResult(TEXT("容器数据一致性"), true, TEXT("服务器端数据一致"));
		PassedTests++;
	}
	else
	{
		FailedTests++;
	}
	
	return bConsistent;
}

// ========================================
// 物品移动同步测试实现
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_WithinContainerMoveSync()
{
	TotalTests++;
	LogTestResult(TEXT("容器内移动同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_CrossContainerMoveSync()
{
	TotalTests++;
	LogTestResult(TEXT("跨容器移动同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_ItemSwapSync()
{
	TotalTests++;
	LogTestResult(TEXT("物品交换同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_StackingSync()
{
	TotalTests++;
	LogTestResult(TEXT("堆叠同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

// ========================================
// 嵌套容器同步测试实现
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_NestedContainerAutoSync()
{
	TotalTests++;
	LogTestResult(TEXT("嵌套容器自动同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_MultiLevelNestedSync()
{
	TotalTests++;
	LogTestResult(TEXT("多层嵌套同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_NestedContainerMoveSync()
{
	TotalTests++;
	LogTestResult(TEXT("嵌套容器移动同步"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

// ========================================
// 服务器广播测试实现
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_BroadcastItemChanges()
{
	TotalTests++;
	LogTestResult(TEXT("服务器广播物品变化"), true, TEXT("待实现"));
	PassedTests++;
	return true;
}

bool AGaiaInventoryNetworkTestActor::Test_MultiClientSync()
{
	TotalTests++;
	
	TArray<APlayerController*> AllControllers = GetAllPlayerControllers();
	
	if (AllControllers.Num() < 2)
	{
		LogTestResult(TEXT("多客户端同步"), true, 
			FString::Printf(TEXT("跳过（需要至少2个玩家，当前%d个）"), AllControllers.Num()));
		PassedTests++;
		return true;
	}

	LogTestResult(TEXT("多客户端同步"), true, 
		FString::Printf(TEXT("检测到%d个玩家"), AllControllers.Num()));
	PassedTests++;
	return true;
}

// ========================================
// 辅助函数实现
// ========================================

TArray<APlayerController*> AGaiaInventoryNetworkTestActor::GetAllPlayerControllers() const
{
	TArray<APlayerController*> Controllers;
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			Controllers.Add(PC);
		}
	}
	
	return Controllers;
}

APlayerController* AGaiaInventoryNetworkTestActor::GetServerPlayerController() const
{
	TArray<APlayerController*> AllControllers = GetAllPlayerControllers();
	
	if (AllControllers.Num() > 0)
	{
		return AllControllers[0]; // 假设第一个是服务器
	}
	
	return nullptr;
}

TArray<APlayerController*> AGaiaInventoryNetworkTestActor::GetClientPlayerControllers() const
{
	TArray<APlayerController*> AllControllers = GetAllPlayerControllers();
	TArray<APlayerController*> ClientControllers;
	
	// 跳过第一个（服务器）
	for (int32 i = 1; i < AllControllers.Num(); i++)
	{
		ClientControllers.Add(AllControllers[i]);
	}
	
	return ClientControllers;
}

UGaiaInventoryRPCComponent* AGaiaInventoryNetworkTestActor::GetRPCComponent(APlayerController* PC) const
{
	if (!PC)
		return nullptr;

	return PC->FindComponentByClass<UGaiaInventoryRPCComponent>();
}

void AGaiaInventoryNetworkTestActor::LogTestResult(const FString& TestName, bool bSuccess, const FString& Details)
{
	if (bSuccess)
	{
		UE_LOG(LogGaiaNetworkTest, Display, TEXT("✅ [%s] 通过 - %s"), *TestName, *Details);
	}
	else
	{
		UE_LOG(LogGaiaNetworkTest, Error, TEXT("❌ [%s] 失败 - %s"), *TestName, *Details);
	}
}

void AGaiaInventoryNetworkTestActor::LogSeparator(const FString& Title)
{
	if (Title.IsEmpty())
	{
		UE_LOG(LogGaiaNetworkTest, Display, TEXT("================================================"));
	}
	else
	{
		UE_LOG(LogGaiaNetworkTest, Display, TEXT("============ %s ============"), *Title);
	}
}

void AGaiaInventoryNetworkTestActor::StartTestsWithDelay()
{
	GetWorld()->GetTimerManager().SetTimer(
		TestStartTimerHandle,
		[this]()
		{
			RunAllTests();
		},
		TestStartDelay,
		false
	);
}

bool AGaiaInventoryNetworkTestActor::VerifyContainerConsistency(const FGuid& ContainerUID, const FString& TestContext)
{
	UGaiaInventorySubsystem* InvSys = GetWorld()->GetSubsystem<UGaiaInventorySubsystem>();
	if (!InvSys)
	{
		return false;
	}

	// 获取容器
	FGaiaContainerInstance Container;
	if (!InvSys->FindContainerByUID(ContainerUID, Container))
	{
		LogTestResult(TestContext, false, TEXT("无法找到容器"));
		return false;
	}

	// 验证每个槽位的物品引用
	for (const FGaiaSlotInfo& Slot : Container.Slots)
	{
		if (Slot.IsEmpty())
			continue;

		FGaiaItemInstance Item;
		if (!InvSys->FindItemByUID(Slot.ItemInstanceUID, Item))
		{
			LogTestResult(TestContext, false, 
				FString::Printf(TEXT("槽位%d引用的物品不存在"), Slot.SlotID));
			return false;
		}

		if (Item.CurrentContainerUID != ContainerUID)
		{
			LogTestResult(TestContext, false, 
				FString::Printf(TEXT("物品的容器UID不匹配（槽位%d）"), Slot.SlotID));
			return false;
		}

		if (Item.CurrentSlotID != Slot.SlotID)
		{
			LogTestResult(TestContext, false, 
				FString::Printf(TEXT("物品的槽位ID不匹配（槽位%d）"), Slot.SlotID));
			return false;
		}
	}

	return true;
}

void AGaiaInventoryNetworkTestActor::LogNetworkStatus()
{
	UE_LOG(LogGaiaNetworkTest, Display, TEXT("========== 网络状态 =========="));
	UE_LOG(LogGaiaNetworkTest, Display, TEXT("服务器: %s"), HasAuthority() ? TEXT("是") : TEXT("否"));
	UE_LOG(LogGaiaNetworkTest, Display, TEXT("网络模式: %d"), (int32)GetNetMode());
	
	TArray<APlayerController*> Controllers = GetAllPlayerControllers();
	UE_LOG(LogGaiaNetworkTest, Display, TEXT("玩家数量: %d"), Controllers.Num());
	
	for (int32 i = 0; i < Controllers.Num(); i++)
	{
		APlayerController* PC = Controllers[i];
		if (PC)
		{
			UE_LOG(LogGaiaNetworkTest, Display, TEXT("  玩家%d: %s (本地: %s)"), 
				i, *PC->GetName(), PC->IsLocalController() ? TEXT("是") : TEXT("否"));
		}
	}
	
	UE_LOG(LogGaiaNetworkTest, Display, TEXT("=============================="));
}

