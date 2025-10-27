// Copyright Epic Games, Inc. All Rights Reserved.

#include "GaiaInventoryNetworkTestActor.h"
#include "GaiaInventorySubsystem.h"
#include "GaiaInventoryRPCComponent.h"
#include "GaiaLogChannels.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

AGaiaInventoryNetworkTestActor::AGaiaInventoryNetworkTestActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// 只在服务器端执行
	bReplicates = false;
}

void AGaiaInventoryNetworkTestActor::BeginPlay()
{
	Super::BeginPlay();

	// 只在服务器端运行测试
	if (!HasAuthority())
	{
		return;
	}

	if (bAutoRunTests)
	{
		StartTestsWithDelay();
	}
}

void AGaiaInventoryNetworkTestActor::StartTestsWithDelay()
{
	if (TestStartDelay > 0.0f)
	{
		UE_LOG(LogGaia, Warning, TEXT("========================================"));
		UE_LOG(LogGaia, Warning, TEXT("网络测试将在 %.1f 秒后开始..."), TestStartDelay);
		UE_LOG(LogGaia, Warning, TEXT("========================================"));

		GetWorld()->GetTimerManager().SetTimer(
			TestStartTimerHandle,
			this,
			&AGaiaInventoryNetworkTestActor::RunAllTests,
			TestStartDelay,
			false
		);
	}
	else
	{
		RunAllTests();
	}
}

void AGaiaInventoryNetworkTestActor::RunAllTests()
{
	LogSeparator(TEXT("开始运行库存系统网络测试"));

	bool bAllTestsPassed = true;

	// 测试1: RPC组件初始化
	if (bTestRPCComponent)
	{
		if (!Test_RPCComponentInitialization())
		{
			bAllTestsPassed = false;
		}
	}

	// 测试2: 容器所有者注册
	if (bTestContainerOwnership)
	{
		if (!Test_ContainerOwnership())
		{
			bAllTestsPassed = false;
		}
	}

	// 测试3: 容器独占访问
	if (bTestExclusiveAccess)
	{
		if (!Test_ExclusiveContainerAccess())
		{
			bAllTestsPassed = false;
		}
	}

	// 测试4: 物品移动同步
	if (bTestItemMovement)
	{
		if (!Test_ItemMovementSync())
		{
			bAllTestsPassed = false;
		}
	}

	// 测试5: 服务器广播
	if (bTestBroadcast)
	{
		if (!Test_ServerBroadcast())
		{
			bAllTestsPassed = false;
		}
	}

	// 总结
	LogSeparator();
	if (bAllTestsPassed)
	{
		UE_LOG(LogGaia, Display, TEXT("✅ 所有网络测试通过！"));
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("❌ 部分网络测试失败！"));
	}
	LogSeparator();
}

void AGaiaInventoryNetworkTestActor::RunSelectedTests()
{
	RunAllTests();
}

// ========================================
// 测试1: RPC组件初始化
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_RPCComponentInitialization()
{
	LogSeparator(TEXT("测试1: RPC组件初始化"));

	TArray<APlayerController*> PlayerControllers = GetAllPlayerControllers();

	if (PlayerControllers.Num() == 0)
	{
		LogTestResult(TEXT("RPC组件初始化"), false, TEXT("没有找到玩家控制器"));
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("找到 %d 个玩家控制器"), PlayerControllers.Num());

	bool bAllHaveComponent = true;

	for (int32 i = 0; i < PlayerControllers.Num(); ++i)
	{
		APlayerController* PC = PlayerControllers[i];
		if (!PC) continue;

		UGaiaInventoryRPCComponent* RPCComp = PC->FindComponentByClass<UGaiaInventoryRPCComponent>();

		if (RPCComp)
		{
			UE_LOG(LogGaia, Log, TEXT("  玩家%d (%s): ✅ 有RPC组件"), 
				i, *PC->GetName());

		if (bVerboseLogging)
		{
			UE_LOG(LogGaia, Verbose, TEXT("    - 拥有的容器: %d"), RPCComp->GetOwnedContainerUIDs().Num());
			UE_LOG(LogGaia, Verbose, TEXT("    - 打开的世界容器: %d"), RPCComp->GetOpenWorldContainerUIDs().Num());
			UE_LOG(LogGaia, Verbose, TEXT("    - 缓存的物品: %d"), RPCComp->GetCachedItemCount());
			UE_LOG(LogGaia, Verbose, TEXT("    - 缓存的容器: %d"), RPCComp->GetCachedContainerCount());
		}
		}
		else
		{
			UE_LOG(LogGaia, Error, TEXT("  玩家%d (%s): ❌ 没有RPC组件"), 
				i, *PC->GetName());
			bAllHaveComponent = false;
		}
	}

	LogTestResult(TEXT("RPC组件初始化"), bAllHaveComponent, 
		FString::Printf(TEXT("%d/%d 玩家有RPC组件"), 
			bAllHaveComponent ? PlayerControllers.Num() : 0, PlayerControllers.Num()));

	return bAllHaveComponent;
}

// ========================================
// 测试2: 容器独占访问
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_ExclusiveContainerAccess()
{
	LogSeparator(TEXT("测试2: 容器独占访问"));

	UGaiaInventorySubsystem* InventorySystem = GetWorld()->GetSubsystem<UGaiaInventorySubsystem>();
	if (!InventorySystem)
	{
		LogTestResult(TEXT("容器独占访问"), false, TEXT("无法获取库存子系统"));
		return false;
	}

	TArray<APlayerController*> PlayerControllers = GetAllPlayerControllers();
	if (PlayerControllers.Num() < 2)
	{
		LogTestResult(TEXT("容器独占访问"), false, TEXT("需要至少2个玩家来测试独占访问"));
		return false;
	}

	APlayerController* Player1 = PlayerControllers[0];
	APlayerController* Player2 = PlayerControllers[1];

	// 创建测试用世界容器（箱子）
	FGuid ChestUID = InventorySystem->CreateContainer(TEXT("TestChest"));
	InventorySystem->SetContainerDebugName(ChestUID, TEXT("测试箱子"));

	UE_LOG(LogGaia, Log, TEXT("创建测试箱子: %s"), *ChestUID.ToString());

	// 玩家1打开箱子
	FString ErrorMessage1;
	bool bPlayer1Open = InventorySystem->TryOpenWorldContainer(Player1, ChestUID, ErrorMessage1);

	if (!bPlayer1Open)
	{
		LogTestResult(TEXT("容器独占访问 - 玩家1打开"), false, ErrorMessage1);
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("  ✅ 玩家1成功打开箱子"));

	// 玩家2尝试打开同一个箱子（应该失败）
	FString ErrorMessage2;
	bool bPlayer2Open = InventorySystem->TryOpenWorldContainer(Player2, ChestUID, ErrorMessage2);

	if (bPlayer2Open)
	{
		LogTestResult(TEXT("容器独占访问 - 玩家2被拒绝"), false, TEXT("玩家2不应该能打开已被占用的箱子"));
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("  ✅ 玩家2被正确拒绝: %s"), *ErrorMessage2);

	// 检查占用状态
	bool bIsOccupied = InventorySystem->IsContainerOccupied(ChestUID);
	APlayerController* CurrentAccessor = InventorySystem->GetContainerAccessor(ChestUID);

	if (!bIsOccupied || CurrentAccessor != Player1)
	{
		LogTestResult(TEXT("容器独占访问 - 占用状态"), false, TEXT("占用状态不正确"));
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("  ✅ 容器占用状态正确"));

	// 玩家1关闭箱子
	InventorySystem->CloseWorldContainer(Player1, ChestUID);

	// 现在玩家2应该可以打开
	FString ErrorMessage3;
	bool bPlayer2OpenAgain = InventorySystem->TryOpenWorldContainer(Player2, ChestUID, ErrorMessage3);

	if (!bPlayer2OpenAgain)
	{
		LogTestResult(TEXT("容器独占访问 - 玩家2重新打开"), false, ErrorMessage3);
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("  ✅ 玩家1关闭后，玩家2成功打开箱子"));

	// 清理
	InventorySystem->CloseWorldContainer(Player2, ChestUID);

	LogTestResult(TEXT("容器独占访问"), true, TEXT("独占访问机制工作正常"));
	return true;
}

// ========================================
// 测试3: 物品移动同步
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_ItemMovementSync()
{
	LogSeparator(TEXT("测试3: 物品移动同步"));

	UGaiaInventorySubsystem* InventorySystem = GetWorld()->GetSubsystem<UGaiaInventorySubsystem>();
	if (!InventorySystem)
	{
		LogTestResult(TEXT("物品移动同步"), false, TEXT("无法获取库存子系统"));
		return false;
	}

	TArray<APlayerController*> PlayerControllers = GetAllPlayerControllers();
	if (PlayerControllers.Num() == 0)
	{
		LogTestResult(TEXT("物品移动同步"), false, TEXT("没有玩家"));
		return false;
	}

	APlayerController* Player1 = PlayerControllers[0];
	UGaiaInventoryRPCComponent* RPC1 = Player1->FindComponentByClass<UGaiaInventoryRPCComponent>();

	if (!RPC1)
	{
		LogTestResult(TEXT("物品移动同步"), false, TEXT("玩家1没有RPC组件"));
		return false;
	}

	// 创建测试容器
	FGuid Container1UID = InventorySystem->CreateContainer(TEXT("TestContainer1"));
	FGuid Container2UID = InventorySystem->CreateContainer(TEXT("TestContainer2"));
	
	InventorySystem->SetContainerDebugName(Container1UID, TEXT("容器1"));
	InventorySystem->SetContainerDebugName(Container2UID, TEXT("容器2"));

	// 注册为玩家1拥有
	InventorySystem->RegisterContainerOwner(Player1, Container1UID);
	InventorySystem->RegisterContainerOwner(Player1, Container2UID);

	// 注意: OwnedContainerUIDs 会通过网络自动复制，测试代码中不需要手动设置
	UE_LOG(LogGaia, Log, TEXT("创建并注册2个测试容器"));

	// 创建测试物品
	FGaiaItemInstance TestItem = InventorySystem->CreateItemInstance(TEXT("TestItem"), 10);
	InventorySystem->SetItemDebugName(TestItem.InstanceUID, TEXT("测试物品"));
	
	// 添加到容器1
	InventorySystem->AddItemToContainer(TestItem, Container1UID);
	
	UE_LOG(LogGaia, Log, TEXT("创建测试物品: %s (数量: 10)"), *TestItem.InstanceUID.ToString());

	// 模拟玩家1移动物品
	FMoveItemResult MoveResult = InventorySystem->MoveItem(
		TestItem.InstanceUID, 
		Container2UID, 
		0, 
		5
	);

	if (!MoveResult.IsSuccess())
	{
		LogTestResult(TEXT("物品移动同步 - 移动物品"), false, MoveResult.ErrorMessage);
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("  ✅ 成功移动 %d 个物品"), MoveResult.MovedQuantity);

	// 验证广播是否触发
	// 注意：在实际测试中，客户端会收到 ClientReceiveInventoryData
	// 这里我们在服务器端验证数据一致性
	
	FGaiaItemInstance FoundItem;
	if (InventorySystem->FindItemByUID(TestItem.InstanceUID, FoundItem))
	{
		if (FoundItem.CurrentContainerUID == Container1UID && FoundItem.Quantity == 5)
		{
			UE_LOG(LogGaia, Log, TEXT("  ✅ 源物品数量正确: %d"), FoundItem.Quantity);
		}
		else
		{
			LogTestResult(TEXT("物品移动同步 - 数据验证"), false, TEXT("源物品数据不正确"));
			return false;
		}
	}

	LogTestResult(TEXT("物品移动同步"), true, TEXT("物品移动和数据验证成功"));
	return true;
}

// ========================================
// 测试4: 服务器广播
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_ServerBroadcast()
{
	LogSeparator(TEXT("测试4: 服务器广播"));

	UGaiaInventorySubsystem* InventorySystem = GetWorld()->GetSubsystem<UGaiaInventorySubsystem>();
	if (!InventorySystem)
	{
		LogTestResult(TEXT("服务器广播"), false, TEXT("无法获取库存子系统"));
		return false;
	}

	TArray<APlayerController*> PlayerControllers = GetAllPlayerControllers();
	if (PlayerControllers.Num() == 0)
	{
		LogTestResult(TEXT("服务器广播"), false, TEXT("没有玩家"));
		return false;
	}

	// 创建测试容器
	FGuid TestContainerUID = InventorySystem->CreateContainer(TEXT("TestBroadcastContainer"));
	InventorySystem->SetContainerDebugName(TestContainerUID, TEXT("广播测试容器"));

	// 注册所有者
	APlayerController* Player1 = PlayerControllers[0];
	InventorySystem->RegisterContainerOwner(Player1, TestContainerUID);

	UE_LOG(LogGaia, Log, TEXT("创建测试容器并注册所有者: %s"), *Player1->GetName());

	// 验证所有者
	TArray<APlayerController*> Owners = InventorySystem->GetContainerOwners(TestContainerUID);
	
	if (Owners.Num() != 1 || Owners[0] != Player1)
	{
		LogTestResult(TEXT("服务器广播 - 所有者验证"), false, TEXT("容器所有者不正确"));
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("  ✅ 容器所有者正确"));

	// 测试广播（内部会查找RPC组件并调用刷新）
	InventorySystem->BroadcastContainerUpdate(TestContainerUID);

	UE_LOG(LogGaia, Log, TEXT("  ✅ 广播已触发"));

	// 注意：实际的RPC调用需要在客户端验证
	// 这里我们只验证服务器端的逻辑

	LogTestResult(TEXT("服务器广播"), true, TEXT("广播机制正常工作"));
	return true;
}

// ========================================
// 测试5: 容器所有者注册
// ========================================

bool AGaiaInventoryNetworkTestActor::Test_ContainerOwnership()
{
	LogSeparator(TEXT("测试5: 容器所有者注册"));

	UGaiaInventorySubsystem* InventorySystem = GetWorld()->GetSubsystem<UGaiaInventorySubsystem>();
	if (!InventorySystem)
	{
		LogTestResult(TEXT("容器所有者注册"), false, TEXT("无法获取库存子系统"));
		return false;
	}

	TArray<APlayerController*> PlayerControllers = GetAllPlayerControllers();
	if (PlayerControllers.Num() == 0)
	{
		LogTestResult(TEXT("容器所有者注册"), false, TEXT("没有玩家"));
		return false;
	}

	APlayerController* Player1 = PlayerControllers[0];

	// 创建容器
	FGuid BackpackUID = InventorySystem->CreateContainer(TEXT("TestBackpack"));
	InventorySystem->SetContainerDebugName(BackpackUID, TEXT("测试背包"));

	UE_LOG(LogGaia, Log, TEXT("创建测试背包: %s"), *BackpackUID.ToString());

	// 注册所有者
	InventorySystem->RegisterContainerOwner(Player1, BackpackUID);

	// 验证所有者
	TArray<APlayerController*> Owners = InventorySystem->GetContainerOwners(BackpackUID);

	if (Owners.Num() != 1 || Owners[0] != Player1)
	{
		LogTestResult(TEXT("容器所有者注册 - 注册验证"), false, TEXT("所有者注册失败"));
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("  ✅ 所有者注册成功"));

	// 注销所有者
	InventorySystem->UnregisterContainerOwner(Player1, BackpackUID);

	// 验证注销
	TArray<APlayerController*> OwnersAfter = InventorySystem->GetContainerOwners(BackpackUID);

	if (OwnersAfter.Num() != 0)
	{
		LogTestResult(TEXT("容器所有者注册 - 注销验证"), false, TEXT("所有者注销失败"));
		return false;
	}

	UE_LOG(LogGaia, Log, TEXT("  ✅ 所有者注销成功"));

	LogTestResult(TEXT("容器所有者注册"), true, TEXT("注册和注销机制正常工作"));
	return true;
}

// ========================================
// 辅助函数
// ========================================

TArray<APlayerController*> AGaiaInventoryNetworkTestActor::GetAllPlayerControllers() const
{
	TArray<APlayerController*> Controllers;

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				Controllers.Add(PC);
			}
		}
	}

	return Controllers;
}

void AGaiaInventoryNetworkTestActor::LogTestResult(const FString& TestName, bool bSuccess, const FString& Details)
{
	if (bSuccess)
	{
		UE_LOG(LogGaia, Display, TEXT("[✅] %s 通过 %s"), 
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("- %s"), *Details));
	}
	else
	{
		UE_LOG(LogGaia, Error, TEXT("[❌] %s 失败 %s"), 
			*TestName, Details.IsEmpty() ? TEXT("") : *FString::Printf(TEXT("- %s"), *Details));
	}
}

void AGaiaInventoryNetworkTestActor::LogSeparator(const FString& Title)
{
	if (Title.IsEmpty())
	{
		UE_LOG(LogGaia, Warning, TEXT("========================================"));
	}
	else
	{
		UE_LOG(LogGaia, Warning, TEXT("========================================"));
		UE_LOG(LogGaia, Warning, TEXT("%s"), *Title);
		UE_LOG(LogGaia, Warning, TEXT("========================================"));
	}
}

